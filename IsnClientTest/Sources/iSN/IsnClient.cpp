/*
 * IsnClient.cpp
 *
 *  Created on: 2017/03/11
 *      Author: Vincent
 */

#include "iSN.h"
#include "UART_Com0.h"
#include "Capteur/FakeCapteur.h"
#include "IsnBuildConfig.h"

/*
 *	IsnClient
 */

IsnClient::IsnClient(Network* n, int device_type)
{
	_net = n;
	_clientStatus = ISN_CLIENTSTATE_NOT_CONNECTED;
	_deviceType = device_type;
	theIsnClient = this;
	_net->setRxHandler(clientMessageHandler);

}

void IsnClient::sendMessage(IsnMessage message)
{
	//Push the message to the end of the Queue, we will send it when ready.
	_sendQueue.push_back(message);
}

void IsnClient::sendMeasure(float m)
{
	//Send a measure to the connected sink
	if (_clientStatus == ISN_CLIENTSTATE_CONNECTED)
	{
		IsnMsgMeasure msg(m);
		sendMessage(msg);
		unicast();
	}
}

//This function is called once in every iteration of the infinite loop.
void IsnClient::exec()
{
	sendRecvMsg();
}

//This function is the bulk of the IsnClient it is called on every iteration
//of the infinite loop. The client decide what to do by looking at its current state.
int IsnClient::sendRecvMsg()
{
	//Return code for this function by default there's no error if an error occur then we change this value.
	int rc = ISN_RC_NO_ERROR;

	//Si le client n'est pas connecté il faut envoyé un search sink
	if (_clientStatus == ISN_CLIENTSTATE_NOT_CONNECTED)
	{
			debug_printf("ISN_CLIENTSTATE_NOT_CONNECTED\n");
			//Ajoute search sink dans la file.
			sendSearchSink();
			//On remet l'adresse du sink a 0;
			_net->resetGwAddress();
			//Envoi le message en diffusion
			rc = broadcast();
			//S'il y a une erreur
			if ( rc != ISN_RC_NO_ERROR){
				//Si ont a toujours pas eu de reponse apres un certain nombre d'essais on revient
				//dans l'etat non connecte
				if (rc == ISN_RC_RETRY_OVER)
					_clientStatus = ISN_CLIENTSTATE_NOT_CONNECTED;
				return rc;
			}
	}

	//What to do when a sink answered with a SEARCH_SINK_ACK
	else if (_clientStatus == ISN_CLIENTSTATE_SINK_FOUND)
	{
		debug_printf("ISN_CLIENTSTATE_SINK_FOUND\n");
		//Ajoute un message connect dans la file d'envoie
		sendConnect();
		rc = unicast();

		if ( rc != ISN_RC_NO_ERROR){
			//Si ont a toujours pas eu de reponse apres un certain nombre d'essais on revient
			//dans l'etat non connecte
			if (rc == ISN_RC_RETRY_OVER)
				_clientStatus = ISN_CLIENTSTATE_NOT_CONNECTED;
			return rc;
		}
	}

	//What to do when the CONFIG frame is received during the connection phase.
	else if (_clientStatus == ISN_CLIENTSTATE_CONFIG_RECEIVED)
	{
		debug_printf("ISN_CLIENTSTATE_CONFIG_RECEIVED\n");
		sendConfigAck();
		unicast();
#ifdef SENSOR_TEMP
		IsnConfigurationTemperature* conf = static_cast<IsnConfigurationTemperature*>(_config);
		//Start a timer for the publication of the sensor's measures.
		_SamplingTimer.start(conf->getSamplingRate() * 1000);
#endif
		_clientStatus = ISN_CLIENTSTATE_CONNECTED;
	}

	else if (_clientStatus == ISN_CLIENTSTATE_CONNECTED)
	{
		//If the publish timer time is up then we must send our measure to the sink.
		if (_SamplingTimer.isTimeUp())
		{
			float measure;
			_sensor->read_sensor(&measure);
			sendMeasure(measure);
#ifdef SENSOR_TEMP
			IsnConfigurationTemperature* conf = static_cast<IsnConfigurationTemperature*>(_config);
			//Restart the timer for the next publish.
			_SamplingTimer.start(conf->getSamplingRate() * 1000);
#endif
		}
	}

	//Read the next frame from the Network receive buffer or do nothing if there's nothing to read.
	_net->readPacket();

	return rc;
}

//This is a wrapper for the UART RX callback internal to the client.
//We register the callback with the Network object but ISO C++ forbid
//passing a pointer to a member function as a callback so we must use
//a global function instead. theIsnClient is a global variable set to point
//to the IsnClient instance. We consider that IsnClient is a singleton.
void clientMessageHandler(tomyClient::NWResponse* resp, int* respCode)
{
	theIsnClient->receiveMessageHandler(resp, respCode);
}

//This function is called when IsnClient receive a Isn Frame on the network.
void IsnClient::receiveMessageHandler(tomyClient::NWResponse* resp, int* respCode)
{
	//Get the type of Isn Message received.
	uint8_t msgType = resp->getIsnType();
	//Get a pointer to the message in the front of the Send Queue or NULL if the Queue is empty
	IsnMessage* msgSend = _sendQueue.frontP();

	printf("Trame Recue:\n");
	debugPrintPayload(resp);

	//Si on recoit un SEARCH_SINK_ACK et qu'on l'attend
	if (msgType == ISN_MSG_SEARCH_SINK_ACK
			&& msgSend != NULL
			&& msgSend->getMessageStatus() == ISN_MSG_STATUS_WAITING
			&& msgSend->getType() == ISN_MSG_SEARCH_SINK)
	{
		//On enregistre l'adresse du sink pour l'unicast
		_net->setGwAddress(resp->getRemoteAddress64());
		//Marque le message SEARCH_SINK comme traite
		msgSend->setMessageStatus(ISN_MSG_STATUS_COMPLETE);
		//Entre dans l'etat ou un sink a ete trouve
		_clientStatus = ISN_CLIENTSTATE_SINK_FOUND;
	}

	//Si on recoit un config suite a un connect et qu'on l'attend
	else if (msgType == ISN_MSG_CONFIG
			&& msgSend != NULL
			&& msgSend->getMessageStatus() == ISN_MSG_STATUS_WAITING
			&& msgSend->getType() == ISN_MSG_CONNECT)
	{
#ifdef SENSOR_TEMP

		//If we compile for the temperature sensor then we create a IsnConfigurationTemperature instance.
			IsnConfigurationTemperature* conf = new IsnConfigurationTemperature(resp->getPayload());
#endif

			if (_config != NULL)
				//We must delete the configuration object before replacing it because it was dynamically created.
				delete _config;
			_config = conf;

		//Marque le message CONNECT comme traite
		msgSend->setMessageStatus(ISN_MSG_STATUS_COMPLETE);



		//Entre dans l'etat CONNECTE
		_clientStatus = ISN_CLIENTSTATE_CONFIG_RECEIVED;
	}

	//Si on recoit un config et qu'on ne l'attend pas alors le serveur
	//n'a pas recu le config ack ou nous envoie une trame de config
	else if (msgType == ISN_MSG_CONFIG
			&& (msgSend == NULL || msgSend->getType() != ISN_MSG_CONNECT))
	{

#ifdef SENSOR_TEMP
		IsnConfigurationTemperature* conf = new IsnConfigurationTemperature(resp->getPayload());
#endif
		if (_config != NULL)
			delete _config;
		_config = conf;
		sendConfigAck();
		unicast();
	}

}

void IsnClient::sendSearchSink()
{
	IsnMsgSearchSink msg(_deviceType);
	sendMessage(msg);
	_clientStatus = ISN_CLIENTSTATE_SEARCH_SENT;
}

void IsnClient::sendConnect()
{
	IsnMsgConnect message;
	sendMessage(message);
	_clientStatus = ISN_CLIENSTATE_CONNECT_SENT;
}

void IsnClient::sendConfigAck()
{
	IsnMsgConfigAck ack;
	sendMessage(ack);
}

int IsnClient::broadcast()
{
    int retry = 0;
    int nbRetry = 0;

    //Prends le message a l'avant de la file
    IsnMessage* msg = _sendQueue.frontP();

    //S'il n'y a pas de message retourne un code d'erreur
    if (msg == NULL)
    	return ISN_RC_NO_MESSAGE;

    nbRetry = msg->getRetry();

    //Essaye d'envoyer le message pour le nombre de retry
    do
    {
    	_net->send(msg->getPayload(), msg->getLength() , BcastReq);

    			int time = msg->getTimeout();
    	        _respTimer.start(time * 1000);
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

    } while(retry++ < nbRetry);

    //On jete le message
    _sendQueue.pop_front();

    return ISN_RC_RETRY_OVER;
}

int IsnClient::unicast()
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

void IsnClient::setSensor(CommonSensor* c)
{
	_sensor = c;
	_sensor->init_sensor();
}

/*
 * Fin IsnClient
 */



