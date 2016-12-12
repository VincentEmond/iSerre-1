/*
 * kinetisClientSample.cpp
 *
 *  Created on: 31 oct. 2016
 *      Author: Julien
 */

#include "mqttsn/MQTTSN_Application.h"

#ifdef KINETIS
#include "mqttsn/mqttsnClientAppFw4kinetis.h"
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




/*------------------------------------------------------
 *             Tasks invoked by Timer
 *------------------------------------------------------*/
bool led_flg = false;

int publier_moyenne(){



}


/*---------  Link Tasks to the Application ----------*/
TASK_LIST = {
	{task1, 1},
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
	{topic1, blinkIndicator, QOS1},
END_OF_SUBSCRIBE_LIST};

/*==================================================
 *      Application setup
 *=================================================*/
 void setup(){
    // nop
 }

#endif



