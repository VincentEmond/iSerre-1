/*
 * IsnServer.cpp
 *
 *  Created on: 2017/03/11
 *      Author: Vincent
 */
#include "iSN.h"

/*
 * IsnServer
 */

IsnServer::IsnServer(Network* net, MqttsnClientApplication* mqtt, int device_type) : _net(net)
{
	theIsnServer = this;
	_serverStatus = ISN_SERVERSTATE_IDLE;
	_deviceType = device_type;
	_net->setRxHandler(serverMessageHandler);
	_mqtt = mqtt;

	TOPIC_TEMP_CAPTEUR = 	new MQString("iserre/temperature/capteur");
	TOPIC_TEMP_ACTION = 	new MQString("iserre/temperature/actionneur");
	TOPIC_TEMP_CONFIG = 	new MQString("iserre/temperature/config");

	TOPIC_HUMID_CAPTEUR = new MQString("iserre/humidite/capteur");
	TOPIC_HUMID_ACTION = 	new MQString("iserre/humidite/actionneur");
	TOPIC_HUMID_CONFIG = 	new MQString("iserre/humidite/config");

	TOPIC_LED_ETAT = 		new MQString("iserre/led/etat");
	TOPIC_LED_INTENSITE = new MQString("iserre/led/intensite");
	TOPIC_LED_COULEUR = 	new MQString("iserre/led/couleur");

	switch (_deviceType)
	{
	case ISN_SENSOR_TEMP:
		_config = new IsnConfigurationTemperature();
	break;
	}


}

void IsnServer::initialize()
{

}

IsnServer::~IsnServer()
{
	delete _config;
}

void IsnServer::sendConfigToAll()
{
	IsnMsgConfig conf = _config->getConfigMsg();
	conf.setTimeout(ISN_SERVER_CONFIG_TIMEOUT);
	conf.setRetry(ISN_SERVER_CONFIG_RETRY);

	vector<IsnClientInfo>::iterator it;

	for (it = _lstClients.begin(); it !=_lstClients.end(); it++)
	{
		NWAddress64 addr = it->getClientAddress();

		_net->setGwAddress(addr);
		sendMessage(conf);
		int rc = unicast();

		//Si on a pas de reponse du client alors on le considere mort.
		if (rc != ISN_RC_NO_ERROR)
			_lstClients.erase(it);
	}

}



void IsnServer::setAndSendConfiguration(IsnConfiguration* config)
{
	delete _config;
	_config = config;
	_serverStatus = ISN_SERVERSTATE_SEND_CONFIG;
}

void serverMessageHandler(tomyClient::NWResponse* resp, int* respCode)
{
	theIsnServer->receiveMessageHandler(resp, respCode);
}


void IsnServer::exec()
{
	sendRecvMsg();
}

int IsnServer::sendRecvMsg()
{
	int rc = ISN_RC_NO_ERROR;

	if (_serverStatus == ISN_SERVERSTATE_IDLE)
	{
		/*vector<IsnClientInfo>::iterator it;

			if (!_lstClients.empty())
			{
				for (it = _lstClients.begin(); it != _lstClients.end();  it++)
				{
					IsnClientInfo inf = *it;
					NWAddress64 addr = inf.getClientAddress();
					_net->setGwAddress(addr);
					sendSearchAck();
					unicast();
				}
			}
		*/

	}

	else if (_serverStatus == ISN_SERVERSTATE_HANDLE_CONNECT)
	{
		IsnClientInfo infos(_net->getGwAddress());

		printf("ISN_SERVERSTATE_HANDLE_CONNECT\n");

		sendConfig();
		int rc = unicast();

		if (rc == ISN_RC_NO_ERROR)
		{
			if (!isAlreadyInList(infos))
				_lstClients.push_back(infos);
		}

		_serverStatus = ISN_SERVERSTATE_IDLE;
	}

	else if (_serverStatus == ISN_SERVERSTATE_HANDLE_SEARCH)
	{
		printf("ISN_SERVERSTATE_HANDLE_SEARCH\n");

		sendSearchAck();
		unicast();
		_serverStatus = ISN_SERVERSTATE_IDLE;
	}

	else if (_serverStatus == ISN_SERVERSTATE_HANDLE_MEASURE)
	{
		printf("ISN_SERVERSTATE_HANDLE_MEASURE\n");
		char str[20] = {0};
		ftoa(_measure, str, 2);
		_mqtt->publish(TOPIC_TEMP_CAPTEUR, str, strlen(str), QOS1);
		_serverStatus = ISN_SERVERSTATE_IDLE;
	}

	else if (_serverStatus == ISN_SERVERSTATE_SEND_CONFIG)
	{
		printf("ISN_SERVERSTATE_SEND_CONFIG\n");
		sendConfigToAll();
		_serverStatus = ISN_SERVERSTATE_IDLE;
	}

	_net->readPacket();

	return rc;
}

bool IsnServer::isAlreadyInList(IsnClientInfo& item)
{
	vector<IsnClientInfo>::iterator it;

	if (_lstClients.empty())
		return false;

	for (it = _lstClients.begin(); it != _lstClients.end();  it++)
	{
		if (item == *it)
			return true;
	}

	return false;
}

void IsnServer::receiveMessageHandler(tomyClient::NWResponse* resp, int* respCode)
{
	uint8_t type = resp->getIsnType();
	IsnMessage* current = _sendQueue.frontP();
	bool sameAddress = false;

	if (current != NULL)
		sameAddress = _net->getGwAddress() == resp->getRemoteAddress64();


	printf("Trame Recue:\n");
	debugPrintPayload(resp);



	if (type == ISN_MSG_SEARCH_SINK)
	{
		//Il faut que le search sois pour le meme type de capteur
		int dt = resp->getPayload(1);

		if (dt == _deviceType)
		{
			_net->setGwAddress(resp->getRemoteAddress64());
			_serverStatus = ISN_SERVERSTATE_HANDLE_SEARCH;
		}

	}

	else if (type == ISN_MSG_CONFIG_ACK &&
			current != NULL &&
			current->getType() == ISN_MSG_CONFIG &&
			current->getMessageStatus() == ISN_MSG_STATUS_WAITING &&
			sameAddress)
	{
		current->setMessageStatus(ISN_MSG_STATUS_COMPLETE);
	}

	else if (type == ISN_MSG_CONNECT)
	{
		_net->setGwAddress(resp->getRemoteAddress64());
		_serverStatus = ISN_SERVERSTATE_HANDLE_CONNECT;
	}

	else if (type == ISN_MSG_MEASURE)
	{
		IsnMsgMeasure msg(resp->getPayload());
		float m = msg.getMeasure();
		_measure = m;
		_serverStatus = ISN_SERVERSTATE_HANDLE_MEASURE;
	}
}

void IsnServer::sendConfigAck()
{
	IsnMsgConfigAck ack;
	sendMessage(ack);
}

void IsnServer::sendSearchAck()
{
	IsnMsgSearchSinkAck ack;
	sendMessage(ack);
}

void IsnServer::sendConfig()
{
	IsnMsgConfig conf = _config->getConfigMsg();
	conf.setRetry(ISN_SERVER_CONFIG_RETRY);
	conf.setTimeout(ISN_SERVER_CONFIG_TIMEOUT);
	sendMessage(conf);
}

void IsnServer::sendMessage(IsnMessage message)
{
	_sendQueue.push_back(message);
}

int IsnServer::unicast()
{
	 int retry = 0;

	    //Prends le message a l'avant de la file
	    IsnMessage* msg = _sendQueue.frontP();

	    //S'il n'y a pas de message retourne un code d'erreur
	    if (msg == NULL)
	    	return ISN_RC_NO_MESSAGE;

	    msg->printPayload();

	    //Essaye d'envoyer le message pour le nombre de retry
	    do
	    {
	    	_net->send(msg->getPayload(), msg->getLength() , UcastReq);

	    	        _respTimer.start(msg->getTimeout() * 1000);
	    	        msg->setMessageStatus(ISN_MSG_STATUS_WAITING);

	    	        while(!_respTimer.isTimeUp()){
	    	        	if(_net->readPacket() != PACKET_ERROR_NODATA){
	    					if (msg->getMessageStatus() == ISN_MSG_STATUS_COMPLETE){
	    						_sendQueue.pop_front();
	    						return ISN_RC_NO_ERROR;
	    					}
	    	        	}
	    	        }
	    	        msg->setMessageStatus(ISN_MSG_STATUS_SEND_REQ);

	    } while(retry++ < msg->getRetry());

	    //On jete le message
		_sendQueue.pop_front();

	    return ISN_RC_RETRY_OVER;
}

int IsnServer::broadcast()
{
    int retry = 0;

    //Prends le message a l'avant de la file
    IsnMessage* msg = _sendQueue.frontP();

    //S'il n'y a pas de message retourne un code d'erreur
    if (msg == NULL)
    	return ISN_RC_NO_MESSAGE;

    //Essaye d'envoyer le message pour le nombre de retry
    do
    {
    	_net->send(msg->getPayload(), msg->getLength() , BcastReq);

    	        _respTimer.start(msg->getTimeout() * 1000);
    	        msg->setMessageStatus(ISN_MSG_STATUS_WAITING);

    	        while(!_respTimer.isTimeUp()){
    	        	if(_net->readPacket() != PACKET_ERROR_NODATA){
    					if (msg->getMessageStatus() == ISN_MSG_STATUS_COMPLETE){
    						_sendQueue.pop_front();
    						return ISN_RC_NO_ERROR;
    					}
    	        	}
    	        }
    	        msg->setMessageStatus(ISN_MSG_STATUS_SEND_REQ);

    } while(retry++ < msg->getRetry());

    //On jete le message
    _sendQueue.pop_front();

    return ISN_RC_RETRY_OVER;
}





/*
 * Fin IsnServer
 */


