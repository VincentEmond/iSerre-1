/*
 * NetworkCallback.cpp
 *
 *  Created on: 2017/02/28
 *      Author: Vincent
 */
#include "receiveHandler.h"

NetworkCallback ReceiveHandler::_mqttCallback = NULL;
NetworkCallback ReceiveHandler::_isnCallback = NULL;

NetworkCallback ReceiveHandler::getReceiveHandler()
{
	return &mqtt_isn_rx_handler;
}

void ReceiveHandler::setIsnCallback(NetworkCallback cb)
{
	_isnCallback = cb;
}

void ReceiveHandler::setMqttCallback(NetworkCallback cb)
{
	_mqttCallback = cb;
}

void ReceiveHandler::mqtt_isn_rx_handler(NWResponse* resp, int* respCode)
{
	if (isIsnData(resp))
		_isnCallback(resp,respCode);
	else
		_mqttCallback(resp, respCode);
}

bool ReceiveHandler::isIsnData(NWResponse*& resp)
{
	uint16_t length = resp->getBodyLength();
	uint8_t* body = resp->getBody();

	if (length < 2)
		return false;

	if (body[0] == 0x02 && body[1] == 0x03) //0x02 MSB(MagicNumber) 0x03 LSB(MagicNumber)
		return true;

	return false;
}



