/*
 * IsnClient.cpp
 *
 *  Created on: 2017/03/11
 *      Author: Vincent
 */

#include "iSN.h"
#include "UART_Com0.h"
#include "Capteur/FakeCapteur.h"

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
	_sendQueue.push_back(message);
}

void IsnClient::sendMeasure(float m)
{
	if (_clientStatus == ISN_CLIENTSTATE_CONNECTED)
	{
		IsnMsgMeasure msg(m);
		sendMessage(msg);
		unicast();
	}
}

void IsnClient::exec()
{
	sendRecvMsg();
}

int IsnClient::sendRecvMsg()
{
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

	else if (_clientStatus == ISN_CLIENTSTATE_CONNECTED)
	{
		debug_printf("ISN_CLIENTSTATE_CONNECTED\n");
	}

	_net->readPacket();

	return rc;
}

void clientMessageHandler(tomyClient::NWResponse* resp, int* respCode)
{
	theIsnClient->receiveMessageHandler(resp, respCode);
}

void IsnClient::receiveMessageHandler(tomyClient::NWResponse* resp, int* respCode)
{
	uint8_t msgType = resp->getIsnType();
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

	//Si on recoit un config suite a un connect qu'on l'attend
	else if (msgType == ISN_MSG_CONFIG
			&& msgSend != NULL
			&& msgSend->getMessageStatus() == ISN_MSG_STATUS_WAITING
			&& msgSend->getType() == ISN_MSG_CONNECT)
	{

		switch (_deviceType)
		{
		case ISN_SENSOR_TEMP:
			IsnConfigurationTemperature conf(resp->getPayload());
			_capteur->setConfig(&conf);
			break;
		}

		//Marque le message CONNECT comme traite
		msgSend->setMessageStatus(ISN_MSG_STATUS_COMPLETE);

		sendConfigAck();
		unicast();

		//Entre dans l'etat CONNECTE
		_clientStatus = ISN_CLIENTSTATE_CONNECTED;
	}

	//Si on recoit un config et qu'on ne l'attend pas alors le serveur
	//n'a pas recu le config ack ou nous envoie une trame de config
	else if (msgType == ISN_MSG_CONFIG
			&& (msgSend == NULL || msgSend->getType() != ISN_MSG_CONNECT))
	{

		switch (_deviceType)
		{
		case ISN_SENSOR_TEMP:
			IsnConfigurationTemperature conf(resp->getPayload());
			_capteur->setConfig(&conf);
			break;
		}
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

void IsnClient::setCapteur(Capteur* c)
{
	_capteur = c;
}

/*
 * Fin IsnClient
 */



