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

IsnMsgSearchSink::IsnMsgSearchSink(uint8_t device_type)
{
	_length = 4; //Magic number 2 byte + Msg type 1 byte + Device Type 1 byte
	_buffer = new uint8_t[_length];
	_deviceType = device_type;
	_buffer[0] = MSB16(ISN_MAGIC_NUMBER);
	_buffer[1] = LSB16(ISN_MAGIC_NUMBER);
	_buffer[2] = ISN_MSG_SEARCH_SINK;
	_buffer[3] = device_type;
}

IsnMsgSearchSinkAck::IsnMsgSearchSinkAck(tomyClient::NWAddress64& sink_address) : _sinkAddress(sink_address)
{
	_length = 2 + 1 + (64/8); //Magic number + msg type + 64 bits address
	_buffer = new uint8_t[_length];
	_buffer[0] = MSB16(ISN_MAGIC_NUMBER);
	_buffer[1] = LSB16(ISN_MAGIC_NUMBER);
	_buffer[2] = ISN_MSG_SEARCH_SINK_ACK;
	uint32_t upper = sink_address.getMsb();
	uint32_t lower = sink_address.getLsb();
	writeDataToBuffer<uint32_t>(&upper, _buffer, 3);
	writeDataToBuffer<uint32_t>(&lower, _buffer, 7);
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


uint8_t* IsnMessage::getPayload()
{
	return _buffer;
}

uint8_t IsnMessage::getLength()
{
	return _length;
}

IsnMessage::IsnMessage() {}

IsnMessage::~IsnMessage()
{
	free(_buffer);
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

void iSN_Start()
{
	tomyClient::NWAddress64 addrSink(0x0013A200, 0x4103DD61);
	IsnMsgSearchSinkAck sink(addrSink);
	sink.printPayload();
}

IsnServer::IsnServer()
{
	_lstClients = new tomyClient::NWAddress64[ISN_SINK_MAX_CLIENT](); //Les () initialise a 0
	theIsnServer = this;
}

void isnRxCallback(tomyClient::NWResponse* resp, int* respCode)
{
	theIsnServer->receiveMessageHandler(resp, respCode);
}


void IsnServer::exec()
{
	printf("iSN server doing work\n");
}

void IsnServer::receiveMessageHandler(tomyClient::NWResponse* resp, int* respCode)
{
	uint8_t isnMsgType = resp->getIsnType();

		switch(isnMsgType)
		{
			case ISN_MSG_SEARCH_SINK:
				printf("Recu search sink");
				addToClientList(resp->getRemoteAddress64());
				break;
			default:
				printf("Erreur. Pas une trame iSN");
		}
}

NetworkCallback IsnServer::getInternalNetworkCallback()
{
	return isnRxCallback;
}

int IsnServer::addToClientList(tomyClient::NWAddress64& addr)
{
	for (int i=0; i<ISN_SINK_MAX_CLIENT; i++)
	{
		if (&(_lstClients[i]) == NULL) {

			tomyClient::NWAddress64* theClient = new tomyClient::NWAddress64();
			memcpy(theClient, &addr, sizeof(tomyClient::NWAddress64));
			_lstClients[i] = *theClient;
			return 0;
		}
	}

	return ISN_ERR_MAX_CLIENT;
}

IsnServer::~IsnServer()
{
	delete[] _lstClients;
}

