/*
 * iSN.cpp
 *
 *  Created on: 2017/02/27
 *      Author: Vincent
 */
#include "iSN.h"
#include "utility/conversion.h"

inline uint8_t MSB16(int x)
{
	return (x >> 8) & 0xFF;
}

inline uint8_t LSB16(int x)
{
	return (x & 0xFF);
}

IsnMsgMeasure::IsnMsgMeasure(float m)
{
	_measure = m;
	_length = 5;
	_type = ISN_MSG_MEASURE;
	_buffer = new uint8_t[_length];
	_buffer[0] = _type;
	writeDataToBuffer<float>(&m,_buffer, 1);
}

float IsnMsgMeasure::getMeasure()
{
	float m = bytesToFloat(&(_buffer[1]));
	return m;
}

IsnMsgConfigAck::IsnMsgConfigAck()
{
	_length = 1; //Frame_Type
	_buffer = new uint8_t[_length];
	_buffer[0] = ISN_MSG_CONFIG_ACK;
	_type = ISN_MSG_CONFIG_ACK;
	_msgStatus = ISN_MSG_STATUS_SEND_REQ;
}

IsnMsgSearchSink::IsnMsgSearchSink(uint8_t device_type)
{
	_length = 2; //Frame_type + Device_type
	_buffer = new uint8_t[_length];
	_deviceType = device_type;
	_buffer[0] = ISN_MSG_SEARCH_SINK;
	_buffer[1] = device_type;
	_timeout = ISN_CLIENT_SEARCH_TIMEOUT;
	_nbRetry = ISN_CLIENT_SEARCH_RETRY;
	_type = ISN_MSG_SEARCH_SINK;
	_msgStatus = ISN_MSG_STATUS_SEND_REQ;
}

IsnMsgSearchSinkAck::IsnMsgSearchSinkAck()
{
	_length = 1; //Frame_Type
	_buffer = new uint8_t[_length];
	_buffer[0] = ISN_MSG_SEARCH_SINK_ACK;
	_type = ISN_MSG_SEARCH_SINK_ACK;
	_msgStatus = ISN_MSG_STATUS_SEND_REQ;
}

IsnMsgConnect::IsnMsgConnect() : IsnMessage()
{
	_length = 1;
	_buffer = new uint8_t[_length];
	_buffer[0] = ISN_MSG_CONNECT;
	_type = ISN_MSG_CONNECT;
	_timeout = ISN_CLIENT_CONNECT_TIMEOUT;
	_nbRetry = ISN_CLIENT_CONNECT_RETRY;
	_msgStatus = ISN_MSG_STATUS_SEND_REQ;
}

IsnMsgPing::IsnMsgPing()
{
	_length = 1;
	_buffer = new uint8_t[_length];
	_buffer[0] = ISN_MSG_PING;
	_type = ISN_MSG_PING;
	_timeout = ISN_CLIENT_CONNECT_TIMEOUT;
	_nbRetry = ISN_CLIENT_CONNECT_RETRY;
	_msgStatus = ISN_MSG_STATUS_SEND_REQ;
}

IsnMsgNotConnected::IsnMsgNotConnected()
{
	_length = 1;
	_buffer = new uint8_t[_length];
	_buffer[0] = ISN_MSG_NOT_CONNECTED;
	_type = ISN_MSG_NOT_CONNECTED;
	_msgStatus = ISN_MSG_STATUS_SEND_REQ;
}

IsnMsgPingAck::IsnMsgPingAck()
{
	_length = 1;
	_buffer = new uint8_t[_length];
	_buffer[0] = ISN_MSG_PING_ACK;
	_type = ISN_MSG_PING_ACK;
	_msgStatus = ISN_MSG_STATUS_SEND_REQ;
}

template<typename T> void writeDataToBuffer(T* data, uint8_t* buffer, int startIndex)
{
	uint8_t* dataBytes = (uint8_t*)data;

	//Le byte order est little endian donc on doit passer les octets en ordre inverse.
	for (int i=startIndex, j=sizeof(T)-1; j >= 0; i++, j--)
	{
		buffer[i] = dataBytes[j];
	}
}

/*
 * IsnMessage
 */

uint8_t* IsnMessage::getPayload()
{
	return _buffer;
}

uint8_t IsnMessage::getLength()
{
	return _length;
}

uint16_t IsnMessage::getTimeout()
{
	return _timeout;
}

void IsnMessage::setTimeout(uint16_t t)
{
	_timeout = t;
}

void IsnMessage::setMessageStatus(uint8_t status)
{
	_msgStatus = status;
}

uint8_t IsnMessage::getMessageStatus()
{
	return _msgStatus;
}

uint8_t IsnMessage::getRetry()
{
	return _nbRetry;
}

IsnMessage::IsnMessage()
{
	_nbRetry = 0;
	_timeout = 0;
	_buffer = NULL;
	_length = 0;
	_msgStatus = ISN_MSG_STATUS_SEND_REQ;
	_type = 0;
}

IsnMessage::IsnMessage(const IsnMessage& msg)
{
	_nbRetry = msg._nbRetry;
	_timeout = msg._timeout;
	_buffer = new uint8_t[msg._length];

	for (int i=0; i<msg._length; i++)
		_buffer[i] = msg._buffer[i];

	_length = msg._length;
	_msgStatus = msg._msgStatus;
	_type = msg._type;
}

IsnMessage::~IsnMessage()
{
	delete[] _buffer;
}

void IsnMessage::printPayload()
{
	printf("==================================\n");
	printf("Donnees de la trame iSN\n");

	for (int i=0; i<_length; i++)
	{
		printf("%02X", _buffer[i]);
		printf(" ");
	}
	printf("\n");
	printf("==================================\n");
}

void debugPrintPayload(tomyClient::NWResponse* resp)
{
	uint8_t* ptr = resp->getFrameDataPtr();
	int length = resp->getFrameLength();

	printf("==================================\n");
	printf("Donnees de la trame iSN\n");



	for (int i=0; i<length; i++)
	{
		printf("%02X", ptr[i]);
		printf(" ");
	}
	printf("\n");
	printf("==================================\n");
}

uint8_t IsnMessage::getType()
{
	return _type;
}

/*
 * IsnMessage end
 */

/*
 * ClientInfo
 */

IsnClientInfo::IsnClientInfo(NWAddress64 addr)
{
	_addr = addr;
}

void IsnClientInfo::setClientAddress(NWAddress64 addr)
{
	_addr = addr;
}

NWAddress64 IsnClientInfo::getClientAddress()
{
	return _addr;
}


bool IsnClientInfo::operator==(IsnClientInfo& other)
{
	return (_addr.getLsb() == other.getClientAddress().getLsb()
			&& _addr.getMsb() == other.getClientAddress().getMsb());
}

/*
 * Fin ClientInfo
 */

/*
 * IsnConfig
 */

IsnConfiguration::~IsnConfiguration() {}
IsnConfigurationTemperature::~IsnConfigurationTemperature() {}

IsnConfigurationTemperature::IsnConfigurationTemperature()
{
	_samplingRate = 30;
	_length = 2;
	_samplingDelay = 5;
}

IsnConfiguration::IsnConfiguration()
{
	_samplingRate = 30;
	_length = 2;
	_samplingDelay = 5;
}


IsnConfiguration::IsnConfiguration(uint8_t* buffer)
{
	uint8_t count = buffer[1];
	int offset = 2;

	for (int i=0; i<count; i++)
	{
		uint8_t type = buffer[i*3+offset];
		uint16_t val = buffer[i*3+offset+1];
		val <<= 8;
		val |= buffer[i*3+offset+2];

		if (type == ISN_CONFIG_TEMP_SAMPLING)
			_samplingRate = val;
		else if (type == ISN_CONFIG_SAMPLING_DELAY)
			_samplingDelay = val;
	}
}

void IsnConfiguration::setSamplingRate(uint16_t sr)
{
	_samplingRate = sr;
}

uint16_t IsnConfiguration::getSamplingRate()
{
	return _samplingRate;
}

uint16_t IsnConfiguration::getSamplingDelay()
{
	return _samplingDelay;
}

void IsnConfiguration::setSamplingDelay(uint16_t dl)
{
	_samplingDelay = dl;
}

IsnMsgConfig IsnConfiguration::getConfigMsg()
{
	IsnConfigParam params[_length];

	params[0].code = ISN_CONFIG_TEMP_SAMPLING;
	params[0].value = _samplingRate;
	params[1].code = ISN_CONFIG_SAMPLING_DELAY;
	params[1].value = _samplingDelay;

	IsnMsgConfig msg(params, _length);

	return msg;
}

IsnConfigParam::IsnConfigParam(uint8_t c, uint16_t v)
{
	code = c;
	value = v;
}

IsnConfigParam::IsnConfigParam() {}

IsnMsgConfig::IsnMsgConfig(IsnConfigParam* params, uint8_t count)
{
	_length = 1 + 1 + count * 3; //Type message + count + params 3 bytes par param
	_buffer = new uint8_t[_length];
	_buffer[0] = ISN_MSG_CONFIG;
	_buffer[1] = count;

	for (int i=0; i<count; i++)
	{
		int offset = i*3+2;
		_buffer[offset] = params[i].code;
		writeDataToBuffer<uint16_t>(&(params[i].value), _buffer, offset+1);
	}
}

string getClientStatusString(int status)
{

	switch (status)
	{
	case ISN_CLIENTSTATE_NOT_CONNECTED:
		return "ISN_CLIENTSTATE_NOT_CONNECTED";
		break;
	case ISN_CLIENSTATE_CONNECT_SENT:
		return "ISN_CLIENSTATE_CONNECT_SENT";
		break;
	case ISN_CLIENTSTATE_CONNECTED:
		return "ISN_CLIENTSTATE_CONNECTED";
		break;
	case ISN_CLIENTSTATE_SEARCH_SENT:
		return "ISN_CLIENTSTATE_SEARCH_SENT";
		break;
	case ISN_CLIENTSTATE_SINK_FOUND:
		return "ISN_CLIENTSTATE_SINK_FOUND";
		break;
	case ISN_CLIENTSTATE_CONFIG_RECEIVED:
		return "ISN_CLIENTSTATE_CONFIG_RECEIVED";
		break;
	default:
		return "";
	}
}

string getMessageString(uint8_t message)
{

	switch (message)
	{
	case ISN_MSG_SEARCH_SINK:
		return "ISN_MSG_SEARCH_SINK";
	case ISN_MSG_SEARCH_SINK_ACK:
		return "ISN_MSG_SEARCH_SINK_ACK";
	case ISN_MSG_CONFIG:
		return "ISN_MSG_CONFIG";
	case ISN_MSG_COMMAND:
		return "ISN_MSG_COMMAND";
	case ISN_MSG_MEASURE:
		return "ISN_MSG_MEASURE";
	case ISN_MSG_CONNECT:
		return "ISN_MSG_CONNECT";
	case ISN_MSG_CONFIG_ACK:
		return "ISN_MSG_CONFIG_ACK";
	case ISN_MSG_NOT_CONNECTED:
		return "ISN_MSG_NOT_CONNECTED";
	case ISN_MSG_PING:
		return "ISN_MSG_PING";
	case ISN_MSG_PING_ACK:
		return "ISN_MSG_PING_ACK";
	default:
		return "";
	}
}




