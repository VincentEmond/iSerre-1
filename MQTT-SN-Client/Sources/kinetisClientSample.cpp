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
		"Noeud6",       //NodeId
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
MQString* topic1 = new MQString("tp1");


/*------------------------------------------------------
 *             Tasks invoked by Timer
 *------------------------------------------------------*/
bool led_flg = false;

int task1(){
	if(led_flg){
		PUBLISH(topic1, "off", 3, QOS1);
		led_flg = false;
	}else{
		PUBLISH(topic1, "on", 2, QOS1);
		led_flg = true;
	}
	return 0;
}

/*---------  Link Tasks to the Application ----------*/
TASK_LIST = {
	{task1, 5},
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



