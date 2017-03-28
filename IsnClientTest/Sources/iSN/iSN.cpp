/*
 * iSN.cpp
 *
 *  Created on: 2017/02/27
 *      Author: Vincent
 */
#include "iSN.h"

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
	_length = 1;
}

IsnConfigurationTemperature::IsnConfigurationTemperature(uint8_t* buffer)
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

void IsnConfigurationTemperature::setSamplingRate(uint16_t sr)
{
	_samplingRate = sr;
}

uint16_t IsnConfigurationTemperature::getSamplingRate()
{
	return _samplingRate;
}

uint16_t IsnConfigurationTemperature::getSamplingDelay()
{
	return _samplingDelay;
}

void IsnConfigurationTemperature::setSamplingDelay(uint16_t dl)
{
	_samplingDelay = dl;
}

IsnMsgConfig IsnConfigurationTemperature::getConfigMsg()
{
	IsnConfigParam params[_length];

	params[0].code = ISN_CONFIG_TEMP_SAMPLING;
	params[0].value = _samplingRate;

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


