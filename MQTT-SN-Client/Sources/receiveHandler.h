/*
 * receiveHandler.h
 *
 *  Created on: 2017/02/28
 *      Author: Vincent
 */

#ifndef SOURCES_RECEIVEHANDLER_H_
#define SOURCES_RECEIVEHANDLER_H_

#include "mqttsn/network.h"
#include "iSN/iSN.h"

using namespace tomyClient;

/*
 * Ce callback va servir a trier les trames et les envoyer a mqtt ou a isn selon le contenu.
 * Mqtt avait le controle total du reseau mais en utilisant cette classe on
 * peut externaliser le controle sans changer quoi que ce sois dans le client mqtt
 */

typedef void(*NetworkCallback)(NWResponse*, int*);

class ReceiveHandler
{
public:
	ReceiveHandler();
	static void setMqttCallback(NetworkCallback);
	static void setIsnCallback(NetworkCallback);
	static NetworkCallback getReceiveHandler();
private:
	static NetworkCallback _mqttCallback;
	static NetworkCallback _isnCallback;
	static void mqtt_isn_rx_handler(NWResponse* resp, int* respCode);
	static bool isIsnData(NWResponse*& resp);
};



#endif /* SOURCES_RECEIVEHANDLER_H_ */
