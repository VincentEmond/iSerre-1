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
#include "utility/stringUtility.h"
#include "utility/conversion.h"
#include "mqttsn\mqttsnClientAppFw4kinetis.h"

using namespace std;
using namespace tomyClient;

//Les messages ISN
#define ISN_MSG_SEARCH_SINK 		0x01
#define ISN_MSG_SEARCH_SINK_ACK 	0x05
#define ISN_MSG_CONFIG				0x02
#define ISN_MSG_COMMAND				0x03
#define ISN_MSG_MEASURE				0x04
#define ISN_MSG_CONNECT				0x07
#define ISN_MSG_CONFIG_ACK			0x06

//Les types de matériel ISN
#define ISN_SENSOR_TEMP				0x01
#define ISN_SENSOR_HUMI				0x02
#define ISN_ACTI_TEMP				0x81
#define ISN_ACTI_HUMI				0x82

//Les codes de configuration
#define ISN_CONFIG_TEMP_SAMPLING	0x01

//Les etats du client
#define ISN_CLIENTSTATE_NOT_CONNECTED 	0x00
#define ISN_CLIENSTATE_CONNECT_SENT		0x01
#define ISN_CLIENTSTATE_CONNECTED		0x02
#define ISN_CLIENTSTATE_SEARCH_SENT		0x03
#define ISN_CLIENTSTATE_SINK_FOUND		0x04

//Les etats du serveur
#define ISN_SERVERSTATE_IDLE			0x01
#define ISN_SERVERSTATE_HANDLE_CONNECT	0x02
#define ISN_SERVERSTATE_HANDLE_SEARCH	0x03
#define ISN_SERVERSTATE_HANDLE_MEASURE  0x04
#define ISN_SERVERSTATE_SEND_CONFIG		0x05

//Timeout du client en s
#define ISN_CLIENT_SEARCH_TIMEOUT		10
#define ISN_CLIENT_CONNECT_TIMEOUT		30

//Timeout du serveur en s

#define ISN_SERVER_CONFIG_TIMEOUT		10

//Retry du client
#define ISN_CLIENT_CONNECT_RETRY 	5
#define ISN_CLIENT_SEARCH_RETRY		5

//Retry du serveur
#define ISN_SERVER_CONFIG_RETRY		5

//Les statut du message
#define ISN_MSG_STATUS_SEND_REQ		0x01
#define ISN_MSG_STATUS_WAITING		0x02
#define ISN_MSG_STATUS_COMPLETE		0x03

//Code de retour lors de l'envoi reception de message
#define ISN_RC_NO_MESSAGE	0x01
#define ISN_RC_NO_ERROR 	0x02
#define ISN_RC_RETRY_OVER	0x03

typedef void(*NetworkCallback)(tomyClient::NWResponse*, int*);

uint8_t MSB16(uint16_t x);
uint8_t LSB16(uint16_t x);

void isnRxCallback(tomyClient::NWResponse* resp, int* respCode);

void debugPrintPayload(tomyClient::NWResponse* resp);

class IsnMessage
{
public:
	IsnMessage();
	~IsnMessage();
	IsnMessage(const IsnMessage&);
	uint8_t* getPayload();
	uint8_t getLength();
	void printPayload();
	uint16_t getTimeout();
	void setTimeout(uint16_t timeout);
	uint8_t getRetry();
	void setRetry(uint8_t r);
	uint8_t getMessageStatus();
	void setMessageStatus(uint8_t);
	uint8_t getType();
protected:
	uint8_t* _buffer;
	uint8_t _length;
	uint16_t _timeout;
	uint8_t _nbRetry;
	uint8_t _msgStatus;
	uint8_t _type;
};

class IsnConfigParam
{
public:
	IsnConfigParam();
	IsnConfigParam(uint8_t c, uint16_t v);
	uint8_t code;
	uint16_t value;
};

class IsnMsgConfig : public IsnMessage
{
public:
	IsnMsgConfig(IsnConfigParam* params, uint8_t count);
};

class IsnConfiguration
{
public:
	virtual IsnMsgConfig getConfigMsg() = 0;
	virtual ~IsnConfiguration();
protected:
	uint8_t _length;
};

class IsnConfigurationTemperature : public IsnConfiguration
{
public:
	IsnConfigurationTemperature();
	IsnMsgConfig getConfigMsg();
	void setSamplingRate(uint16_t sr);
	uint16_t getSamplingRate();
	~IsnConfigurationTemperature();
private:
	uint16_t _samplingRate;
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
public:
	IsnMsgSearchSinkAck();
};

class IsnMsgCommand : public IsnMessage
{

};

class IsnMsgMeasure : public IsnMessage
{
public:
	IsnMsgMeasure(uint8_t* buffer);
	float getMeasure();
};

class IsnMsgConnect : public IsnMessage
{
public:
	IsnMsgConnect();
};

class IsnMsgConfigAck : public IsnMessage
{
public:
	IsnMsgConfigAck();
};

template <class T>
class Queue
{
public:
	bool empty();
	int size();
	T front();
	T back();
	void push_back(T element);
	void pop_front();
	T* frontP();
private:
	vector<T> _list;
};

/*
 * Queue
 */

template<class T>
bool Queue<T>::empty()
{
	return _list.empty();
}

template<class T>
int Queue<T>::size()
{
	return _list.size();
}

template<class T>
T Queue<T>::front()
{
	return _list.front();
}

template<class T>
T* Queue<T>::frontP()
{
	if (this->empty())
		return NULL;

	return &(*_list.begin());
}

template<class T>
T Queue<T>::back()
{
	return _list.back();
}

template<class T>
void Queue<T>::push_back(T item)
{
	_list.push_back(item);
}

template<class T>
void Queue<T>::pop_front()
{
	if (!_list.empty())
	{
		_list.erase(_list.begin());
	}
}
/*
 * Fin Queue
 */

//Utiliser pour encapsuler les info sur le client dans la liste de clients du serveur
class IsnClientInfo
{
public:
	NWAddress64 getClientAddress();
	void setClientAddress(NWAddress64 addr);
	IsnClientInfo(NWAddress64 addr);
	bool operator==(IsnClientInfo& other);
private:
	NWAddress64 _addr;
};

class IsnServer
{
public:
	IsnServer(Network* net, MqttsnClientApplication* mqtt, int device_type);
	~IsnServer();
	void exec();
	void receiveMessageHandler(tomyClient::NWResponse* resp, int* respCode);
	void setAndSendConfiguration(IsnConfiguration* config);
	void initialize();
private:
	int _serverStatus;
	vector<IsnClientInfo> _lstClients;
	int sendRecvMsg();
	XTimer _respTimer;
	Network* _net;
	Queue<IsnMessage> _sendQueue;
	IsnConfiguration* _config;
	bool isAlreadyInList(IsnClientInfo&);
	void sendMessage(IsnMessage message);
	void sendSearchAck();
	void sendConfigAck();
	void sendConfig();
	void sendConfigToAll();
	int unicast();
	int broadcast();
	int _deviceType;
	MqttsnClientApplication* _mqtt;
	float _measure;
	MQString* TOPIC_TEMP_CAPTEUR;
	MQString* TOPIC_TEMP_ACTION;
	MQString* TOPIC_TEMP_CONFIG;
	MQString* TOPIC_HUMID_CAPTEUR;
	MQString* TOPIC_HUMID_ACTION;
	MQString* TOPIC_HUMID_CONFIG;
	MQString* TOPIC_LED_ETAT;
	MQString* TOPIC_LED_INTENSITE;
	MQString* TOPIC_LED_COULEUR;

};

class IsnClient
{
public:
	IsnClient(Network* n, int device_type);
	void exec();
	void receiveMessageHandler(tomyClient::NWResponse* resp, int* respCode);
private:
	int _clientStatus;
	int _deviceType;
	Network* _net;
	Queue<IsnMessage> _sendQueue;
	void sendMessage(IsnMessage message);
	void sendSearchSink();
	void sendConnect();
	int broadcast();
	int unicast();
	int sendRecvMsg();
	XTimer _respTimer;
};


void clientMessageHandler(tomyClient::NWResponse* resp, int* respCode);
void serverMessageHandler(tomyClient::NWResponse* resp, int* respCode);

static IsnServer* theIsnServer;
static IsnClient* theIsnClient;

template<typename T> void writeDataToBuffer(T* data, uint8_t* buffer, int startIndex);



#endif /* SOURCES_ISN_H_ */
