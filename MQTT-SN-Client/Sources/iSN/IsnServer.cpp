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

IsnServer::IsnServer(Network& net) : _net(net)
{
	theIsnServer = this;
	_serverStatus = ISN_SERVERSTATE_IDLE;
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
		printf("iSN Server Idle\n");
	}

	else if (_serverStatus == ISN_SERVERSTATE_HANDLE_CONNECT)
	{
		IsnClientInfo infos(_net.getGwAddress());

		if (!isAlreadyInList(infos))
			_lstClients.push_back(infos);

		sendConnectAck();
		unicast();
		_serverStatus = ISN_SERVERSTATE_IDLE;
	}

	else if (_serverStatus == ISN_SERVERSTATE_HANDLE_SEARCH)
	{
		sendSearchAck();
		unicast();
		_serverStatus = ISN_SERVERSTATE_IDLE;
	}

	_net.readPacket();

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

	if (type == ISN_MSG_SEARCH_SINK)
	{
		_net.setGwAddress(resp->getRemoteAddress64());
		_serverStatus = ISN_SERVERSTATE_HANDLE_SEARCH;
	}

	else if (type == ISN_MSG_CONNECT)
	{
		_net.setGwAddress(resp->getRemoteAddress64());
		_serverStatus = ISN_SERVERSTATE_HANDLE_CONNECT;
	}
}

void IsnServer::sendConnectAck()
{
	IsnMsgConnectAck ack;
	sendMessage(ack);
}

void IsnServer::sendSearchAck()
{
	IsnMsgSearchSinkAck ack;
	sendMessage(ack);
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

	    //Essaye d'envoyer le message pour le nombre de retry
	    do
	    {
	    	_net.send(msg->getPayload(), msg->getLength() , UcastReq);

	    	        _respTimer.start(msg->getTimeout() * 1000);
	    	        msg->setMessageStatus(ISN_MSG_STATUS_WAITING);

	    	        while(!_respTimer.isTimeUp()){
	    	        	if(_net.readPacket() != PACKET_ERROR_NODATA){
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
    	_net.send(msg->getPayload(), msg->getLength() , BcastReq);

    	        _respTimer.start(msg->getTimeout() * 1000);
    	        msg->setMessageStatus(ISN_MSG_STATUS_WAITING);

    	        while(!_respTimer.isTimeUp()){
    	        	if(_net.readPacket() != PACKET_ERROR_NODATA){
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


