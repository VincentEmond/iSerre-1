/*
 * iSN-Serveur.h
 *
 *  Created on: 2017/02/27
 *      Author: Vincent
 */

#ifndef SOURCES_ISN_H_
#define SOURCES_ISN_H_

#include "mqttsn/zbeeStack.h"
#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include <vector>

using namespace std;
using namespace tomyClient;

//Les messages ISN
#define ISN_MSG_SEARCH_SINK 		0x01
#define ISN_MSG_SEARCH_SINK_ACK 	0x05
#define ISN_MSG_CONFIG				0x02
#define ISN_MSG_COMMAND				0x03
#define ISN_MSG_MEASURE				0x04

//Les types de matériel ISN
#define ISN_SENSOR_TEMP				0x01
#define ISN_SENSOR_HUMI				0x02
#define ISN_ACTI_TEMP				0x81
#define ISN_ACTI_HUMI				0x82

//Les etats du client
#define ISN_CLIENTSTATE_NOT_CONNECTED 	0x00
#define ISN_CLIENSTATE_CONNECT_SENT		0x01
#define ISN_CLIENTSTATE_CONNECTED		0x02

//Les etats du serveur
#define ISN_SERVERSTATE_IDLE			0x01
#define ISN_SERVERSTATE_HANDLE_CONNECT	0x02

//Timeout du client en ms
#define ISN_CLIENT_SEARCH_TIMEOUT		10000
#define ISN_CLIENT_CONNECT_TIMEOUT		30000

//Retry du client
#define ISN_CLIENT_CONNECT_RETRY 5

typedef void(*NetworkCallback)(tomyClient::NWResponse*, int*);

uint8_t MSB16(uint16_t x);
uint8_t LSB16(uint16_t x);

void isnRxCallback(tomyClient::NWResponse* resp, int* respCode);



class IsnMessage
{
public:
	IsnMessage();
	~IsnMessage();
	uint8_t* getPayload();
	uint8_t getLength();
	void printPayload();
protected:
	uint8_t* _buffer;
	uint8_t _length;
};

class IsnMsgSearchSink : public IsnMessage
{
public:
	IsnMsgSearchSink(uint8_t device_type);
private:
	uint8_t _deviceType;
};

class IsnMsgSearchSinkAck : public IsnMessage
{

};

class IsnMsgConfig : public IsnMessage
{

};

class IsnMsgCommand : public IsnMessage
{

};

class IsnMsgMeasure : public IsnMessage
{

};

class IsnServer
{
public:
	IsnServer(Network net);
	~IsnServer();
	void exec();
	void receiveMessageHandler(tomyClient::NWResponse* resp, int* respCode);
	NetworkCallback getInternalNetworkCallback();
	int addToClientList(tomyClient::NWAddress64& addr);
private:
	vector<tomyClient::NWAddress64> _lstClients;
	Network _net;
};

class IsnClient
{
public:
	IsnClient(Network n);
	void exec();
private:
	int _clientStatus;
	Network _net;
};

static IsnServer* theIsnServer;

template<typename T> void writeDataToBuffer(T* data, uint8_t* buffer, int startIndex);



#endif /* SOURCES_ISN_H_ */
