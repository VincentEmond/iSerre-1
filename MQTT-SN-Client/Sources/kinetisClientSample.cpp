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
#include "utility/StringUtilCpp.h"
#include "iSN/iSN.h"

#define SINK_TEMP
//#define SINK_HUMI
//#define SINK_ACTI

#define DEFAULT_MEASURE_NUMBER (4u)
#define DEFAULT_READ_FRAME_PERIOD (10u)

using namespace std;
using namespace tomyClient;

/*============================================
 *
 *   MQTT-SN Client Application for mbed
 *
 *===========================================*/
XBeeAppConfig theAppConfig = {
	{
		57600,          //Baudrate (bps)
		0,              //Serial PortNo
		0               //Device (for linux App)
	},
	{
#if defined(SINK_TEMP)
		"SinkTemp",     //NodeId
#elif defined(SINK_HUMI)
		"SinkHumi",
#elif defined(SINK_ACTI)
		"SinkActi",
#endif
		300,            //KeepAlive (sec)
		false,          //Clean session
		false,          //EndDevice
		0,    			//WillTopic   or 0   DO NOT USE 0 STRING!
		0   			//WillMessage or 0   DO NOT USE 0 STRING!
	}
 };

Network* pNetwork = new Network(UART1_IDX, &UART_Com1_initConfig, &UART_Com1_state);

Network& network = *pNetwork;

/*------------------------------------------------------
 *             Xbee module address
 *------------------------------------------------------*/
//NWAddress64 tempActuator(0x0013A200, 0x4103DD61);
//NWAddress64 ledActuator (0x0013A200, 0x40C1C448);

/*------------------------------------------------------
 *             Create Topic
 *------------------------------------------------------*/

MQString* TOPIC_TEMP_CAPTEUR = 	new MQString("iserre/temperature/capteur");
MQString* TOPIC_TEMP_ACTION = 	new MQString("iserre/temperature/actionneur");
MQString* TOPIC_TEMP_CONFIG = 	new MQString("iserre/temperature/config");

MQString* TOPIC_HUMID_CAPTEUR = new MQString("iserre/humidite/capteur");
MQString* TOPIC_HUMID_ACTION = 	new MQString("iserre/humidite/actionneur");
MQString* TOPIC_HUMID_CONFIG = 	new MQString("iserre/humidite/config");

MQString* TOPIC_LED_ETAT = 		new MQString("iserre/led/etat");
MQString* TOPIC_LED_INTENSITE = new MQString("iserre/led/intensite");
MQString* TOPIC_LED_COULEUR = 	new MQString("iserre/led/couleur");

#ifndef SINK_ACTI
SinkMeasureManager measMgr(DEFAULT_MEASURE_NUMBER);
#endif



/*------------------------------------------------------
 *             Tasks invoked by Timer
 *------------------------------------------------------*/
#ifndef SINK_ACTI
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
#endif

#ifdef SINK_TEMP

int readTempFrame(){
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

#endif

#ifdef SINK_HUMI

int readHumiFrame(){
	debug_printf("reading...\n\r");
	network.readPacket();
	debug_printf("done\n\r");

	if(measMgr.isAverageDone())
	{
		char str[20] = {0};
		ftoa(measMgr.getAverage(), str, 2);
		PUBLISH(TOPIC_HUMI_CAPTEUR, str, strlen(str), QOS1);
	}

	return 0;
}

#endif

#ifdef SINK_ACTI

#endif

/*---------  Link Tasks to the Application ----------*/
TASK_LIST = {

#ifdef SINK_TEMP
	{readTempFrame, DEFAULT_READ_FRAME_PERIOD},
#endif

#ifdef SINK_HUMI
	{readHumiFrame, DEFAULT_READ_FRAME_PERIOD},
#endif

#ifdef SINK_ACTI

#endif

END_OF_TASK_LIST};

/*------------------------------------------------------
 *       Tasks invoked by PUBLISH command Packet
 *------------------------------------------------------*/

#ifdef SINK_TEMP
// Defines all tasks invoked by PUBLISH command packet for temperature sensor
int tempConfigCallback(MqttsnPublish* msg){
	string text(reinterpret_cast<char*>(msg->getData()));
	vector<string> params = split(text, ";");
	IsnConfigurationTemperature* tempConf = new IsnConfigurationTemperature();

	for (vector<string>::iterator it = params.begin(); it != params.end(); it++)
	{
		string nameVal = *it;
		vector<string> nameValSplit = split(nameVal, "=");

		string name = nameValSplit[0];
		string val = nameValSplit[1];
		uint16_t numericValue;

		if (name == "ISN_CONFIG_TEMP_SAMPLING")
		{

			numericValue = strtoul(val.c_str(), NULL, 10);
			tempConf->setSamplingRate(numericValue);
		}
	}



	isnSrv->setAndSendConfiguration(tempConf);

	return 0;
}
#endif

#ifdef SINK_HUMI
// Defines all tasks invoked by PUBLISH command packet for humidity sensor
int humiConfigCallback(MqttsnPublish* msg){
	return 0;
}
#endif

#ifdef SINK_ACTI
// Defines all tasks invoked by PUBLISH command packet for actuator control

int tempActionneurCallback(MqttsnPublish* msg){

	if( !strncmp("1", (const char*)msg->getData(),1)){
		debug_printf("led on\n\r");
		network.setGwAddress(tempActuator);
		uint8_t frame[] = {iSN_FrameType_Command, 1};
		network.send(frame, sizeof(frame), UcastReq);
	}else if( !strncmp("0", (const char*)msg->getData(),1)){
		debug_printf("led off\n\r");
		network.setGwAddress(tempActuator);
		uint8_t frame[] = {iSN_FrameType_Command, 0};
		network.send(frame, sizeof(frame), UcastReq);
	}
	return 0;
}

int humiActionneurCallback(MqttsnPublish* msg){

	/*if( !strncmp("1", (const char*)msg->getData(),1)){
		debug_printf("led on\n\r");
		network.setGwAddress(tempActuator);
		uint8_t frame[] = {iSN_FrameType_Command, 1};
		network.send(frame, sizeof(frame), UcastReq);
	}else if( !strncmp("0", (const char*)msg->getData(),1)){
		debug_printf("led off\n\r");
		network.setGwAddress(tempActuator);
		uint8_t frame[] = {iSN_FrameType_Command, 0};
		network.send(frame, sizeof(frame), UcastReq);
	}*/
	return 0;
}

int ledActionneurCallback(MqttsnPublish* msg){

	//debug_printf("receive led PUBSLISH\n\r");
	network.setGwAddress(ledActuator);
	uint8_t *frame = new uint8_t[msg->getDataLength()];
	memcpy(frame, msg->getData(), msg->getDataLength());
	debug_printf("frame : %s, size = %u\n\r", frame, msg->getDataLength());
	network.send(frame, msg->getDataLength(), UcastReq);
	delete frame;
	return 0;
}
#endif
/*-------------- Link Tasks to Topic -------------------*/
SUBSCRIBE_LIST = {
#ifdef SINK_TEMP
// register all tasks invoked by PUBLISH command packet for temperature sensor
	{TOPIC_TEMP_CONFIG, tempConfigCallback, QOS1},
#endif
#ifdef SINK_HUMI
// register all tasks invoked by PUBLISH command packet for humidity sensor
	{TOPIC_TEMP_CONFIG, humiConfigCallback, QOS1},
#endif
#ifdef SINK_ACTI
// register all tasks invoked by PUBLISH command packet for temperature sensor
	{TOPIC_TEMP_ACTION, tempActionneurCallback, QOS1},
	{TOPIC_HUMID_ACTION, humiActionneurCallback, QOS1},
	{TOPIC_LED_ETAT, ledActionneurCallback, QOS1},
	{TOPIC_LED_COULEUR, ledActionneurCallback, QOS1},
	{TOPIC_LED_INTENSITE, ledActionneurCallback, QOS1},
#endif
END_OF_SUBSCRIBE_LIST};

/*==================================================
 *      Application setup
 *=================================================*/
 void setup(){
	 debug_printf("start\n\r");
	 network.initialize(theAppConfig.netCfg);
#ifndef SINK_ACTI
	 //network.setRxHandler(networkRxCallback);
#endif
 }

#endif



