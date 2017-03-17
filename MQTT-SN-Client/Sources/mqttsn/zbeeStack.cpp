/*
 * zbeeStack.cpp
 *
 *                      The BSD License
 *
 *           Copyright (c) 2014, tomoaki@tomy-tech.com
 *                    All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Created on: 2014/06/01
 *    Modified: 2014/09/05
 *      Author: Tomoaki YAMAGUCHI
 *     Version: 1.0.0
 */

#ifndef ARDUINO
        #include "MQTTSN_Application.h"
		#include "Network.h"
#else
        #include <MQTTSN_Application.h>
		#include <Network.h>
#endif

#ifdef NETWORK_XBEE

#ifdef ARDUINO
  #include <zbeeStack.h>
  #include <mqUtil.h>

  #if defined(MQTTSN_DEBUG) || defined(NW_DEBUG) || defined(DEBUG)
	#include <SoftwareSerial.h>
	extern SoftwareSerial debug;
  #endif

#endif  /* ARDUINO */

#ifdef MBED
        #include "mbed.h"
        #include "zbeeStack.h"
		#include "mqUtil.h"
#endif /* MBED */

#ifdef LINUX
        #include "zbeeStack.h"
		#include "mqUtil.h"
        #include <stdio.h>
        #include <sys/time.h>
        #include <sys/types.h>
        #include <sys/stat.h>
        #include <unistd.h>
        #include <stdlib.h>
        #include <string.h>
        #include <fcntl.h>
        #include <errno.h>
        #include <termios.h>

#endif /* LINUX */

#ifdef KINETIS
		//#include "UART_Com2.h"

#endif /* KINETIS */

using namespace std;
using namespace tomyClient;

extern uint16_t getUint16(uint8_t* pos);
extern uint32_t getUint32(uint8_t* pos);
extern void setUint16(uint8_t* pos, uint16_t val);
extern void setUint32(uint8_t* pos, uint32_t val);

/*=========================================
       Class SerialPort
 =========================================*/
#ifdef ARDUINO
/**
 *  For Arduino
 */
SerialPort::SerialPort(){
	_serialDev = 0;
	pinMode(XB_SLEEP_PIN, OUTPUT);
	digitalWrite(XB_SLEEP_PIN, LOW);
}

int SerialPort::open(XBeeConfig config){ //Port num overload.
	if (config.portNo == 0){
	Serial.begin(config.baudrate);
	_serialDev = (Stream*) &Serial;
	}
	#if defined(UBRR1H)
	else if (config.portNo == 1){
	Serial1.begin(config.baudrate);
	_serialDev = (Stream*) &Serial1;
	}
	#endif
	#if defined(UBRR2H)
	else if (config.portNo == 2){
	Serial2.begin(config.baudrate);
	_serialDev = (Stream*) &Serial2;
	}
	#endif
	#if defined(UBRR3H)
	else if (config.portNo == 3){
	Serial3.begin(config.baudrate);
	_serialDev = (Stream*) &Serial3;
	}
	#endif
	return 0;
}

bool SerialPort::checkRecvBuf(){
    return _serialDev->available() > 0;
}

bool SerialPort::send(unsigned char b){
	while(true){
		if(digitalRead(XB_CTS_PIN) == LOW){
			break;
		}
	}
	if(_serialDev->write(b) != 1){
	  return false;
	}else{
	  D_NWSTACK(" ");
	  D_NWSTACK(b,HEX);
	  return true;
	}
}


bool SerialPort::recv(unsigned char* buf){
    if ( _serialDev->available() > 0 ){
        buf[0] = _serialDev->read();

        D_NWSTACK(" ");
        D_NWSTACK(*buf,HEX);
        return true;

    }else{
        return false;
    }
}

void SerialPort::flush(void){
    _serialDev->flush();
}

#endif /* ARDUINO */

#ifdef MBED
/**
 *  For MBED
 */
SerialPort::SerialPort(){
  _serialDev = new Serial(ZB_MBED_SERIAL_TXPIN, ZB_MBED_SERIAL_RXPIN);
  _head = _tail = 0;
}

void SerialPort::setBuff(void){
    while(_serialDev->readable() > 0){
        if( _tail != (_head + 1 == RING_BUFFER_SIZE ? 0 : _head + 1)){  // buffer full
            _data[_head] = _serialDev->getc();
            if( ++_head == RING_BUFFER_SIZE){
                _head = 0;
            }
        }
    }
}

int SerialPort::open(XBeeConfig config){
  _serialDev->baud(config.baudrate);
  _serialDev->format(8,Serial::None,1);
  _serialDev->attach(this, &SerialPort::setBuff,Serial::RxIrq);
  return 0;
}

bool SerialPort::checkRecvBuf(){
    return _head != _tail;
}

bool SerialPort::send(unsigned char b){
	_serialDev->putc(b);
	D_NWSTACKF( " %x", b);
	return true;
}


bool SerialPort::recv(unsigned char* buf){
    if(_head != _tail){
        *buf = _data[_tail];
        D_NWSTACKF( " %x",*buf );
        if(++_tail == RING_BUFFER_SIZE){
            _tail = 0;
        }
        return true;
    }
    *buf = 0;
    return false;
}



void SerialPort::flush(void){
    _head = _tail = 0;
}

#endif /* MBED */

#ifdef KINETIS
/**
 *  For KINETIS
 */
#define XBEE_PACKETIZATION_TIMEOUT (5u)

static void UART_RxCallback(uint32_t instance, void* state)
{
	SerialPort* serial = (SerialPort*)((uart_state_t*)state)->rxCallbackParam;
	serial->setBuff();
}

SerialPort::SerialPort(uint32_t instance, const uart_user_config_t* config, uart_state_t* state)
{
	_instance = instance;

	// init to a known state

	_config.baudRate 			= config->baudRate;
	_config.bitCountPerChar 	= config->bitCountPerChar;
	_config.parityMode 			= config->parityMode;
	_config.stopBitCount 		= config->stopBitCount;

	_state = state;

	UART_DRV_Init(_instance, _state, &_config);

	_begin = _end = _size = 0;
}

void SerialPort::setBuff(void)
{
	_data[_end] = _state->rxBuff[0]; // read the incoming byte
	_size++;

	_end++;
	if(_end == SERIAL_PORT_FIFO_SIZE)
		_end = 0;
}

int SerialPort::open(XBeeConfig config)
{
	UART_DRV_Deinit(_instance);
	_config.baudRate 			= config.baudrate;
	_config.bitCountPerChar 	= kUart8BitsPerChar;
	_config.parityMode 			= kUartParityDisabled;
	_config.stopBitCount 		= kUartOneStopBit;
	UART_DRV_Init(_instance, _state, &_config);

	UART_DRV_InstallRxCallback(_instance, UART_RxCallback, _buffer, (void*)this, true);

	_begin = _end = _size = 0;

	return 0;
}

bool SerialPort::checkRecvBuf()
{
    return _end != _begin;
}

bool SerialPort::send(unsigned char b)
{
	OSA_TimeDelay(XBEE_PACKETIZATION_TIMEOUT * (int)(10. / _config.baudRate) );
	if(UART_DRV_SendData(_instance, &b, 1) == kStatus_UART_Success)
		return true;
	else
		return false;
}

bool SerialPort::recv(unsigned char* buf)
{
	if((_size > SERIAL_PORT_FIFO_SIZE) || (_end == _begin)) // if buffer is full or empty
	{
		*buf = 0;
		return false;
	}
	else
    {
        *buf = _data[_begin]; // copy the first element of the fifo
        _size--;

        _begin++; // shift first element index by 1
        if(_begin == SERIAL_PORT_FIFO_SIZE)
            _begin = 0;

        return true;
    }
}

void SerialPort::flush(void)
{
    _end = _begin = _size = 0; // manually emptying the fifo
}

#endif /* KINETIS */

#ifdef LINUX

SerialPort::SerialPort(){
    _tio.c_iflag = IGNBRK | IGNPAR;
#ifdef XBEE_FLOWCTRL_CRTSCTS
    _tio.c_cflag = CS8 | CLOCAL | CREAD | CRTSCTS;
#else
    _tio.c_cflag = CS8 | CLOCAL | CREAD;
#endif
    _tio.c_cc[VINTR] = 0;
    _tio.c_cc[VTIME] = 0;
    _tio.c_cc[VMIN] = 0;
    _fd = 0;
}

SerialPort::~SerialPort(){
  if (_fd){
      close(_fd);
  }
}

int SerialPort::open(XBeeConfig config){
  return open(config.device, config.baudrate, false, 1);
}

int SerialPort::open(const char* devName, unsigned int boaurate,  bool parity, unsigned int stopbit){
  _fd = ::open(devName, O_RDWR | O_NOCTTY);
  if(_fd < 0){
      return _fd;
  }

  if (parity){
      _tio.c_cflag = _tio.c_cflag | PARENB;
  }
  if (stopbit == 2){
      _tio.c_cflag = _tio.c_cflag | CSTOPB ;
  }
  switch(boaurate){
    case B9600:
    case B19200:
    case B38400:
    case B57600:
    case B115200:
      if( cfsetspeed(&_tio, boaurate)<0){
        return errno;
      }
      break;
    default:
      return -1;
  }
    return tcsetattr(_fd, TCSANOW, &_tio);
}

bool SerialPort::checkRecvBuf(){
    return true;
}

bool SerialPort::send(unsigned char b){
  if (write(_fd, &b,1) != 1){
      return false;
  }else{
      D_NWSTACKF( " %x", b);
      return true;
  }
}

bool SerialPort::recv(unsigned char* buf){
  if(read(_fd, buf, 1) == 0){
      return false;
  }else{
      D_NWSTACKF( " %x",*buf );
      return true;
  }
}

void SerialPort::flush(void){
  tcsetattr(_fd, TCSAFLUSH, &_tio);
}

#endif

/*=========================================
             Class XBeeAddress64
 =========================================*/
NWAddress64::NWAddress64(){
    _msb = _lsb = 0;
}

NWAddress64::NWAddress64(uint32_t msb, uint32_t lsb){
    _msb = msb;
    _lsb = lsb;
}

uint32_t NWAddress64::getMsb(){
    return _msb;
}

uint32_t NWAddress64::getLsb(){
    return _lsb;
}

void NWAddress64::setMsb(uint32_t msb){
    _msb = msb;
}

void NWAddress64::setLsb(uint32_t lsb){
    _lsb = lsb;
}

bool NWAddress64::operator ==(const NWAddress64& other) const
{
	return (_lsb == other._lsb) && (_msb == other._msb);
}

/*=========================================
             Class XBResponse
 =========================================*/
XBResponse::XBResponse(){
    reset();
}

uint8_t XBResponse::getApiId(){
    return _apiId;
}

uint8_t XBResponse::getMsbLength(){
    return _msbLength;
}

uint8_t XBResponse::getLsbLength(){
    return _frameLength;
}

uint8_t XBResponse::getChecksum(){
    return _checksum;
}

uint8_t XBResponse::getFrameLength(){
    return _frameLength;
}

uint8_t* XBResponse::getFrameDataPtr(){
    return _frameDataPtr;
}

void XBResponse::setMsbLength(uint8_t msbLength){
    _msbLength = msbLength;
}

void XBResponse::setLsbLength(uint8_t lsbLength){
	_frameLength = lsbLength;
}

void XBResponse::setChecksum(uint8_t checksum){
    _checksum = checksum;
}

void XBResponse::setFrameDataPtr(uint8_t* frameDataPtr){
	_frameDataPtr = frameDataPtr;
}
void XBResponse::setFrameLength(uint8_t frameLength){
	_frameLength = frameLength;
}

void XBResponse::setApiId(uint8_t api){
    _apiId = api;
}

bool XBResponse::isAvailable(){
    return _complete;
}

void XBResponse::setAvailable(bool complete){
    _complete = complete;
}

bool XBResponse::isError(){
    return _errorCode > 0;
}

uint8_t XBResponse::getErrorCode(){
    return _errorCode;
}

void XBResponse::setErrorCode(uint8_t errorCode){
    _errorCode = errorCode;
}


void XBResponse::reset(){
      _apiId = 0;
      _msbLength = 0;
      _checksum = 0;
      _frameLength = 0;
      _errorCode = NO_ERROR;
      _complete = false;
}

/*=========================================
             Class XBModemStatus
 =========================================*/
XBModemStatus::XBModemStatus(){
}

uint8_t XBModemStatus::getStatus(){
	return *getFrameDataPtr();
}

/*=========================================
             Class NWResponse
 =========================================*/
NWResponse::NWResponse(){
    reset();
    _remoteAddress16 = 0;
    _options = 0;
}

NWAddress64&  NWResponse::getRemoteAddress64(){
    return _remoteAddress64;
}

uint16_t NWResponse::getRemoteAddress16(){
  return _remoteAddress16;
}

void  NWResponse::setRemoteAddress64(NWAddress64& addr64){
    _remoteAddress64 = addr64;
}

void  NWResponse::setRemoteAddress64(){
	_remoteAddress64.setMsb(getUint32(getFrameDataPtr()));
	_remoteAddress64.setLsb(getUint32(getFrameDataPtr() + 4));
}

void  NWResponse::setRemoteAddress16(uint16_t addr16){
    _remoteAddress16 = addr16;
}

void  NWResponse::setRemoteAddress16(){
    _remoteAddress16 = (uint16_t(getFrameDataPtr()[8]) << 8) + (getFrameDataPtr()[9]);
}

uint8_t NWResponse::getType(){
	/*
	 * Selon MQTT-SN: if the first octet of the Length field is coded 窶�x01窶�
	 * then the Length field is 3-octet long; in this case, the two
	 * following octets specify the total number of octets of the message (most-significant octet first)
	 */
	if (_frameDataPtr[ZB_RSP_DATA_OFFSET] == 0x01)
		return _frameDataPtr[ZB_RSP_DATA_OFFSET + 3];
	return _frameDataPtr[ZB_RSP_DATA_OFFSET + 1];
}

uint8_t NWResponse::getIsnType(){
	//Dans iSN le type apparait au dﾃｩbut de la trame.
	return _frameDataPtr[ZB_RSP_DATA_OFFSET];
}

uint8_t NWResponse::getPayload(uint8_t index){
    return _frameDataPtr[index + ZB_RSP_DATA_OFFSET];
}

uint8_t* NWResponse::getBody(){
	if(getFrameLength() > 255){
		return _frameDataPtr + ZB_RSP_DATA_OFFSET + 4;
	}else{
		return _frameDataPtr + ZB_RSP_DATA_OFFSET + 2;
	}
}

uint16_t NWResponse::getBodyLength(){
	if(getFrameLength() > 255){
		return getPayloadLength() - 4;
	}else{
		return getPayloadLength() - 2;
	}
}

uint8_t* NWResponse::getPayload(){
    return _frameDataPtr + ZB_RSP_DATA_OFFSET;
}

uint8_t NWResponse::getPayloadLength(){
    return getFrameLength() - ZB_RSP_DATA_OFFSET;
}

uint8_t NWResponse::getOption(){
    return _options;
}

void NWResponse::setOption(uint8_t options){
    _options = options;
}

void NWResponse::setOption(){
    _options = *(_frameDataPtr + ZB_RSP_DATA_OFFSET - 1);
}

bool NWResponse::isBrodcast(){
    return ( _options && 0x02);
}

/*=========================================
           Class NWRequest
 =========================================*/

NWRequest::NWRequest(){

}

uint8_t NWRequest::getFrameDataLength(){
    return  _payloadLength + ZB_REQ_DATA_OFFSET;
}

uint8_t* NWRequest::getPayload(){
    return _payloadPtr;
}

uint8_t NWRequest::getPayloadLength(){
    return _payloadLength;
}

uint8_t NWRequest::getBroadcastRadius(){
    return _broadcastRadius;
}

uint8_t NWRequest::getOption(){
    return _option;
}

void NWRequest::setBroadcastRadius(uint8_t broadcastRadius){
    _broadcastRadius = broadcastRadius;
}

void NWRequest::setOption(uint8_t option){
    _option = option;
}

void NWRequest::setPayload(uint8_t *payload){
    _payloadPtr = payload;
}

void NWRequest::setPayloadLength(uint8_t payLoadLength){
    _payloadLength = payLoadLength;
}


/*===========================================
              Class  Network
 ============================================*/

Network::Network(uint32_t instance, const uart_user_config_t* config, uart_state_t* state){

	_serialPort = new SerialPort(instance, config, state);
    _rxCallbackPtr = 0;
    _returnCode = 0;
    _response.setFrameDataPtr(_responsePayload);
    _tm.stop();
    _pos = 0;
    _escape = false;
    _checksumTotal = 0;
    _gwAddress64.setMsb(0L);
    _gwAddress64.setLsb(0L);
    _gwAddress16 = 0;
    _sleepflg = false;
}

Network::~Network(){
	delete _serialPort;
}

int Network::initialize(XBeeConfig config){
	return _serialPort->open(config);
}

void Network::setSleep(){
	_sleepflg = true;
}

void Network::setRxHandler(void (*callbackPtr)(NWResponse* data, int* returnCode)){
    _rxCallbackPtr = callbackPtr;
}

NWAddress64& Network::getRxRemoteAddress64(){
    return _rxResp.getRemoteAddress64();
}

uint16_t Network::getRxRemoteAddress16(){
    return _rxResp.getRemoteAddress16();
}

void Network::setResponse(NWResponse& response){
	response.setApiId(_response.getApiId());
	response.setAvailable(_response.isAvailable());
	response.setChecksum(_response.getChecksum());
	response.setErrorCode(_response.getErrorCode());
	response.setFrameLength(_response.getFrameLength());
	response.setMsbLength(_response.getMsbLength());
	response.setLsbLength(_response.getLsbLength());
	response.setFrameDataPtr(_responsePayload);
    response.setOption();
    response.setRemoteAddress16();
    response.setRemoteAddress64();
}


void Network::setGwAddress(){
    _gwAddress64.setMsb(getRxRemoteAddress64().getMsb());
    _gwAddress64.setLsb(getRxRemoteAddress64().getLsb());
    _gwAddress16 = getRxRemoteAddress16();
}

NWAddress64 Network::getGwAddress()
{
	return _gwAddress64;
}

void Network::setGwAddress(NWAddress64& addr){
	_gwAddress64.setMsb(addr.getMsb());
	_gwAddress64.setLsb(addr.getLsb());
	_gwAddress16 = uint16_t(addr.getLsb() & 0xFFFF);
}

void Network::resetGwAddress(void){
    _gwAddress64.setMsb(0);
    _gwAddress64.setLsb(0);
    _gwAddress16 = 0;
}

void Network::setSerialPort(SerialPort *serialPort){
  _serialPort = serialPort;
}


void Network::send(uint8_t* payload, uint8_t payloadLen, SendReqType type){
    _txRequest.setOption(0);
    _txRequest.setPayload(payload);
    _txRequest.setPayloadLength(payloadLen);
    sendZBRequest(_txRequest, type);
}

int Network::readPacket(uint8_t type){
    _returnCode = 0;

    //S'il y a quelque chose dans le tampon de rﾃｩception
    if(_serialPort->checkRecvBuf()){
		if(readApiFrame(PACKET_TIMEOUT_CHECK)){
			if(_response.getApiId() == ZB_API_RESPONSE){
				//Si on a reﾃｧu une trame rﾃｩponse ZigBee et qu'on a
				//settﾃｩ un callback alors on appel ce callback
				if (_rxCallbackPtr != 0){
					_rxCallbackPtr(&_rxResp, &_returnCode);
				}
			}else if(_response.getApiId() == ZB_API_MODEMSTATUS){
				if(type){
					_returnCode = PACKET_MODEM_STATUS;
				}
			}
        }
    }
    return _returnCode;
}

bool Network::readApiFrame(uint16_t timeoutMillsec){
    _pos = 0;
    _tm.start((uint32_t)timeoutMillsec);


    while(!_tm.isTimeUp()){

        readApiFrame();

        if(_response.isAvailable()){
            D_NWSTACKW("\r\n<=== CheckSum OK\r\n\n");
            if(_response.getApiId() == ZB_API_RESPONSE){
            	//_rxResp.setFrameDataPtr(_rxFrameDataBuf);
            	setResponse(_rxResp);
				if(_gwAddress16 &&
					(_rxResp.getOption() & 0x02 ) != 0x02 &&
					(_gwAddress64.getMsb() != _rxResp.getRemoteAddress64().getMsb()) &&
					(_gwAddress64.getLsb() != _rxResp.getRemoteAddress64().getLsb())){
					D_NWSTACKW("  Sender is not Gateway!\r\n" );
					return false;
				}
            	return true;
            }else if(_response.getApiId() == ZB_API_MODEMSTATUS){
            	return true;
            }
        }else if(_response.isError()){
            D_NWSTACKW("\r\n<=== Packet Error Code = ");
            D_NWSTACKLN(_response.getErrorCode(), DEC);
            D_NWSTACKF("%d\r\n",_response.getErrorCode() );
            return false;
        }
    }
    return false;   //Timeout
}

void Network::readApiFrame(){

    if (_response.isAvailable() || _response.isError()){
      resetResponse();
    }

    while(read(&_byteData )){

    	//Si on lit 0x7E c'est qu'on est sur le start delimiter de la trame api.
        if( _byteData == START_BYTE){
            _pos = 1;
            continue;
        }
        // Check ESC

        /*
         * Si on est en API mode = 2. Certains caractﾃｨres peuvent ﾃｪtre ﾃｩchappﾃｩ.
         * Pour ce faire on met 0x7D devant le caractﾃｨre ﾃ�ﾃｩchapper et on le XOR avec 0x20
         * pour "Togglﾃｩ" son bit 5
         */
        if(_pos > 0 && _byteData == ESCAPE){
          if(read(&_byteData )){
              _byteData = 0x20 ^ _byteData;  // decode
          }else{
        	  //Si on a reﾃｧu l'indicateur d'ﾃｩchappement mais que le prochain octet
        	  //n'est pas disponible dans le tampon alors on met un flag a true pour
        	  //traiter le prochain octet reﾃｧu comme un octet ﾃｩchappﾃｩ
              _escape = true;
              continue;
          }
        }

        if(_escape){
            _byteData = 0x20 ^ _byteData;
            _escape = false;
        }

        //On commence ﾃ�accumuler ﾃ�partir du champ Frame Type pour le calcul du checksum
        //ﾃ㌢ veux dire que Start Delimiter et Length sont ignorﾃｩ dans le calcul.
        if(_pos >= API_ID_POS){
            _checksumTotal+= _byteData;
        }

        switch(_pos){
        case 0:
        	//On est sur le start delimiter
            break;

        case 1:
        	//On est sur l'octet le plus significatif de la longueur de la trame.
            _response.setMsbLength(_byteData);
            break;

        case 2:
        	//On est sur l'octet le moins significatif de la longueur de la trame
            _response.setLsbLength(_byteData);
            break;
        case 3:
        	//On est sur l'octet Frame Type qui indique le type de Trame Api.
        	D_NWSTACKW("\r\n===> Recv:    ");
            _response.setApiId(_byteData);   // API
            break;
        default:
        	//Limite la trame ﾃ�la longueur maximale d'une trame MQTT-SN.
        	//Ce comportement fais que l'objet rﾃｩseau n'est plus indﾃｩpendant de MQTT
            if(_pos > MQTTSN_MAX_FRAME_SIZE){
              _response.setErrorCode(PACKET_EXCEEDS_BYTE_ARRAY_LENGTH);
              _pos = 0;
              return;
            //Si on est sur le dernier octet de la trame qui est le Checksum.
            //Note: On ajoute +3 car start delimiter (1 octet) et Lengths (2 octets)
            //Ne font pas partie du length de la trame.
            }else if(_pos == (_response.getFrameLength() + 3)){
            		//On masque pour limiter le checksum ﾃ�1 octet
				  if((_checksumTotal & 0xff) == 0xff){
					  /*
					   * L'ﾃｩmetteur calcul le checksum en accumulant les champs ﾃ�partir de
					   * Frame Type jusqu'ﾃ�checksum exclusivement. Il prends ensuite cette somme
					   * et la maske ﾃ�1 octet (& 0xFF) il va ensuite calculer 0xFF - Somme = Checksum
					   * et placer cette somme dans le champs checksum de maniﾃｨre ﾃ�ce que l'accumulation des champs
					   * de Frame Type ﾃ�checksum inclusivement donne toujours 0xFF.
					   */
					  _response.setChecksum(_byteData);
					  _response.setAvailable(true);
					  _response.setErrorCode(NO_ERROR);
				  }else{
					  _response.setErrorCode(CHECKSUM_FAILURE);
				  }
				  _response.setFrameLength(_pos - 4);    // 4 = 2(packet len) + 1(Api) + 1(checksum)
				  _pos = 0;
				  _checksumTotal = 0;
				  return;
            }else{
            	//L'index 0 de notre trame dﾃｩcapsulﾃｩ commence ﾃ�Frame ID.
            	//On ignore start delimiter (1 octet) + Length (2 octets) + Frame Type (1 octet)
              _response.getFrameDataPtr()[_pos - 4] = _byteData;
            }
            break;
        }
        _pos++;

    }
}

void Network::sendZBRequest(NWRequest& request, SendReqType type){
    D_NWSTACKW("\r\n===> Send:    ");

    sendByte(START_BYTE, false);           // Start byte

    uint8_t msbLen = ((request.getFrameDataLength() + 1) >> 8) & 0xff; // 1  for Checksum
    uint8_t lsbLen = (request.getFrameDataLength() + 1) & 0xff;
    sendByte(msbLen, true);               // Message Length
    sendByte(lsbLen, true);               // Message Length

    sendByte(ZB_API_REQUEST, true);       // API
    uint8_t checksum = 0;
    checksum+= ZB_API_REQUEST;

    sendByte(0x00, true);                 // Frame ID

    for(int i = 0; i < 10; i++){
        sendByte(getAddrByte(i,type), true);   // Gateway Address 64 & 16
        checksum += getAddrByte(i,type);       //   or Broadcast Address
    }

    sendByte(request.getBroadcastRadius(), true);
    checksum += request.getBroadcastRadius();

    sendByte(request.getOption(), true);
    checksum += request.getOption();

    D_NWSTACKW("\r\n     Payload: ");

    for( int i = 0; i < request.getPayloadLength(); i++ ){
        sendByte(request.getPayload()[i], true);     // Payload
        checksum+= request.getPayload()[i];
    }
    checksum = 0xff - checksum;
    sendByte(checksum, true);

    flush();  // clear receive buffer

    D_NWSTACKW("\r\n<=== Send completed\r\n\n" );
}

void Network::sendByte(uint8_t b, bool escape){
  if(escape && (b == START_BYTE || b == ESCAPE || b == XON || b == XOFF)){
      write(ESCAPE);
      write(b ^ 0x20);
  }else{
      write(b);
  }
}

void Network::resetResponse(){
  _pos = 0;
  _escape = 0;
  _response.reset();
  _addr16 = 0;
  _addr32 = 0;
}

void Network::flush(){
  _serialPort->flush();
}

bool Network::write(uint8_t val){
	return (_serialPort->send(val) ? true : false );
}

bool Network::read(uint8_t *buff){
	return  _serialPort->recv(buff);
}

uint8_t Network::getAddrByte(uint8_t pos, SendReqType type){
	uint8_t buf[4];

    if(type == UcastReq){
        if (pos == 0){
			setUint32(buf, _gwAddress64.getMsb());
			return buf[0];
		}else if (pos == 1){
			setUint32(buf, _gwAddress64.getMsb());
			return buf[1];
		}else if (pos == 2){
			setUint32(buf, _gwAddress64.getMsb());
			return buf[2];
		}else if (pos == 3){
			setUint32(buf, _gwAddress64.getMsb());
			return buf[3];
		}else if (pos == 4){
			setUint32(buf, _gwAddress64.getLsb());
			return buf[0];
		}else if (pos == 5){
			setUint32(buf, _gwAddress64.getLsb());
			return buf[1];
		}else if (pos == 6){
			setUint32(buf, _gwAddress64.getLsb());
			return buf[2];
		}else if (pos == 7){
			setUint32(buf, _gwAddress64.getLsb());
			return buf[3];
		}else if (pos == 8){
			setUint16(buf,_gwAddress16);
			return buf[0];
		}else if (pos == 9){
			setUint16(buf,_gwAddress16);
			return buf[1];
		}
    }else if(type == BcastReq){
        if(pos < 6){
            return 0x00;
        }else if((pos > 5) && (pos < 9)){
            return 0xff;
        }else if(pos == 9){
            return 0xfe;
        }
    }
    return 0;
}

#endif  /* NETWORK_XBEE */

