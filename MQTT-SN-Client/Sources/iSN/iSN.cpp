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

IsnMsgConnectAck::IsnMsgConnectAck()
{
	_length = 1; //Frame_Type
	_buffer = new uint8_t[_length];
	_buffer[0] = ISN_MSG_CONNECT_ACK;
	_type = ISN_MSG_CONNECT_ACK;
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


