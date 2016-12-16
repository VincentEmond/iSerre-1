/*
 * kinetisClientSample.cpp
 *
 *  Created on: 31 oct. 2016
 *      Author: Julien
 */

#include "mqttsn/MQTTSN_Application.h"

#ifdef KINETIS
#include "mqttsn/mqttsnClientAppFw4kinetis.h"
#include "mqttsn/zbeeStack.h"
#include "UART_Com1.h"
#include "iSerre-SN.h"
#include "SinkMeasureManager.h"
#include "utility/StringUtility.h"
#include <string.h>

using namespace std;
using namespace tomyClient;

/*============================================
 *
 *   MQTT-SN Client Application for mbed
 *
 *===========================================*/
XBeeAppConfig theAppConfig = {
	{
		57600,           //Baudrate (bps)
		0,              //Serial PortNo
		0               //Device (for linux App)
	},
	{
		"Noeud-CH",       //NodeId
		300,           //KeepAlive (sec)
		false,          //Clean session
		false,          //EndDevice
		0,    			//WillTopic   or 0   DO NOT USE 0 STRING!
		0   			//WillMessage or 0   DO NOT USE 0 STRING!
	}
 };

Network network(UART1_IDX, &UART_Com1_initConfig, &UART_Com1_state);

/*------------------------------------------------------
 *             Create Topic
 *------------------------------------------------------*/
MQString* TOPIC_TEMP_CAPTEUR = new MQString("iserre/temperature/capteur");
MQString* TOPIC_TEMP_ACTION = new MQString("iserre/temperature/actionneur");
MQString* TOPIC_TEMP_CONFIG = new MQString("iserre/temperature/config");

MQString* TOPIC_HUMID_CAPTEUR = new MQString("iserre/humidite/capteur");
MQString* TOPIC_HUMID_ACTION = new MQString("iserre/humidite/actionneur");
MQString* TOPIC_HUMID_CONFIG = new MQString("iserre/humidite/config");

MQString* TOPIC_LUMIERE_CONFIG = new MQString("iserre/lumiere/config");



/*  Logique sink temperature*/

	/** Cote MQTT : Souscriptions **
	 * TOPIC_TEMP_CONFIG : chaque fois qu'un msg est publié enregistrer la config dans var. configCourante;
	 * TOPIC_TEMP_ACTION : chaque fois qu'un msg est publié, envoyer une trame iSN_FrameCommand aux actionneurs.

	 */

	// Deux tâches principales, la premiere ne fait que publier periodiquement (selon configCourante) la var. lastTemp

	/* Tache 1 : Publier sur TOPIC_TEMP_CAPTEUR la dernière moyenne calculée
	 *
	 * Envoyer un iSN_FrameCommand avec valeur 2 (demande mesure) à tous les noeuds capteur
	 *
	 */


	/* Tache 2 : Gestion noeuds iSN */
	/** Boucle principale
		Verifier buffer de reception Xbee_iSN
		Si iSN_FrameSearchSink trouvé
			Enregistrer adresse du noeud émetteur
			Envoyer trame iSN_FrameConfig au noeud émetteur
		Si iSN_FrameMeasure trouvé
			incrémenter compteur de trame de donnees recues : (int) valeurRecues++;
			enregistrer valeur dans tabValeurs[];
			si (valeurRecues == valeurDeMoyenne)
				lastTemp = 0;
				for (valeur dans tabValeurs):
					lastTemp += valeur
				lastTemp = lastTemp / valeurDeMoyenne

	**/

#define DEFAULT_MEASURE_NUMBER (4u)

SinkMeasureManager measMgr(DEFAULT_MEASURE_NUMBER);

void networkRxCallback(NWResponse* data, int* returnCode)
{
	if(data->getPayload(0) == iSN_FrameType_Measure)
	{
		float meas = 0.f;
		convertBuffer2Float(&data->getPayload()[1], &meas);
		debug_printf("measure = %.2f\n\r", meas);

		measMgr.addMeasure(meas);
	}
	network.readPacket();
}

/*------------------------------------------------------
 *             Tasks invoked by Timer
 *------------------------------------------------------*/
int readFrame(){
	debug_printf("reading...\n\r");
	network.readPacket();
	debug_printf("done\n\r");

	if(measMgr.isAverageDone())
	{
		char str[20] = {0};
		ftoa(measMgr.getAverage(), str, 2);
		PUBLISH(TOPIC_TEMP_CAPTEUR, str, strlen(str), QOS1);
	}

	return 0;
}


/*---------  Link Tasks to the Application ----------*/
TASK_LIST = {
	{readFrame, 10},
END_OF_TASK_LIST};

/*------------------------------------------------------
 *       Tasks invoked by PUBLISH command Packet
 *------------------------------------------------------*/

int  blinkIndicator(MqttsnPublish* msg){

  if( !strncmp("on", (const char*)msg->getData(),2)){
    debug_printf("led on\n\r");
  }else if( !strncmp("off", (const char*)msg->getData(),3)){
	  debug_printf("led off\n\r");
  }
  return 0;
}
/*-------------- Link Tasks to Topic -------------------*/
SUBSCRIBE_LIST = {
	//{topic1, blinkIndicator, QOS1},
END_OF_SUBSCRIBE_LIST};

/*==================================================
 *      Application setup
 *=================================================*/
 void setup(){
	 debug_printf("start\n\r");
	 network.initialize(theAppConfig.netCfg);
	 network.setRxHandler(networkRxCallback);
 }

#endif



