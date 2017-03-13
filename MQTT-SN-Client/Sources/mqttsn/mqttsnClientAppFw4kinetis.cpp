/*
 * mqttsnClientAppAppFw4Kinetis.cpp
 *
 *                      The BSD License
 *
 *           Copyright (c) 2014, tomoaki@tomy-tech.com
 *                    All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Created on: 2014/06/01
 *    Modified: 2014/09/05
 *      Author: Tomoaki YAMAGUCHI
 *     Version: 1.0.0
 */

#ifndef ARDUINO

#include "MQTTSN_Application.h"
#ifdef KINETIS
#include "mqttsnClientAppFw4kinetis.h"
#include "mqttsn_client_c_interface.h"

#include "utility/Clock.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "iSN/iSN.h"

using namespace std;
using namespace tomyClient;


extern TaskList theTaskList[];
extern OnPublishList theOnPublishList[];
extern APP_CONFIG    theAppConfig;
extern void  setup();

MqttsnClientApplication* theApplication = new MqttsnClientApplication();

/*========================================
		main function
=========================================*/
int mqttsnClientAppMain()
{

	IsnServer* isnSrv = new IsnServer(pNetwork, ISN_SENSOR_TEMP);


	theApplication->setKeepAlive(theAppConfig.mqttsnCfg.keepAlive);
	theApplication->setClean(theAppConfig.mqttsnCfg.cleanSession);
	if(theAppConfig.mqttsnCfg.willTopic){
		MQString* willTpc = new MQString(theAppConfig.mqttsnCfg.willTopic);
		theApplication->setWillTopic(willTpc);
	}
	if(theAppConfig.mqttsnCfg.willMsg){
		MQString* willMsg = new MQString(theAppConfig.mqttsnCfg.willMsg);
		theApplication->setWillMessage(willMsg);
	}

	theApplication->addTask();
	setup();

	theApplication->initialize(theAppConfig);

	while (true)
	{
		//Run iSN
		isnSrv->exec();
		//Run MQTT-SN
		theApplication->run();
	}


	return 0;
}

/*========================================
		Class MqttsnClientApplication
=========================================*/

MqttsnClientApplication::MqttsnClientApplication(){

}

MqttsnClientApplication::~MqttsnClientApplication(){

}


void MqttsnClientApplication::startWdt(){
    _wdTimer.start();
}

void MqttsnClientApplication::stopWdt(){
    _wdTimer.stop();
}

/*------------ Client execution  forever --------------*/
int MqttsnClientApplication::run(){

	_wdTimer.wakeUp();
	_mqttsn.readPacket();
	int rc = _mqttsn.exec();
	if(rc == MQTTSN_ERR_REBOOT_REQUIRED){
		_mqttsn.subscribe();
	}

	return 0;
}

Network* MqttsnClientApplication::getNetwork()
{
	 return _mqttsn.getNetwork();
}

NetworkCallback MqttsnClientApplication::getInternalNetworkCallback()
{
	return _mqttsn.getInternalNetworkCallback();
}

void MqttsnClientApplication::setExternalNetworkCallback(NetworkCallback cb)
{
	_mqttsn.setExternalNetworkCallback(cb);
}

void MqttsnClientApplication::initialize(APP_CONFIG config){
	_mqttsn.initialize(config);
	setSubscribe();
}

void MqttsnClientApplication::setSubscribe(){
	_mqttsn.setSubscribing(true);  // re-entrant control
	_mqttsn.subscribe();
	_mqttsn.setSubscribing(false);
}

void MqttsnClientApplication::addTask(){
	for(int i = 0; theTaskList[i].sec; i++){
		_wdTimer.registerCallback(theTaskList[i].sec, theTaskList[i].callback);
	}
}

void MqttsnClientApplication::setKeepAlive(uint16_t sec){
    _mqttsn.setKeepAlive(sec);
}


void MqttsnClientApplication::setWillTopic(MQString* willTopic){
    _mqttsn.setWillTopic(willTopic);
}

void MqttsnClientApplication::setWillMessage(MQString* willMsg){
    _mqttsn.setWillMessage(willMsg);
}

void MqttsnClientApplication::setRetain(bool retain){
    _mqttsn.setRetain(retain);
}

void MqttsnClientApplication::setClean(bool clean){
    _mqttsn.setClean(clean);
}

int MqttsnClientApplication::publish(MQString* topic, const char* data, int dataLength, uint8_t qos){
	return _mqttsn.publish(topic, data, dataLength, qos);
}

int MqttsnClientApplication::publish(uint16_t predefinedId, const char* data, int dataLength, uint8_t qos){
    return _mqttsn.publish(predefinedId, data, dataLength, qos);
}

int MqttsnClientApplication::publish(MQString* topic, Payload* payload, uint8_t qos){
	return _mqttsn.publish(topic, payload, qos);
}


int MqttsnClientApplication::subscribe(MQString* topic, TopicCallback callback, uint8_t qos){
    return _mqttsn.subscribe(topic, callback,qos);
}

int MqttsnClientApplication::subscribe(uint16_t predefinedId, TopicCallback callback, uint8_t qos){
    return _mqttsn.subscribe(predefinedId, callback, qos);
}


int MqttsnClientApplication::unsubscribe(MQString* topic){
    return _mqttsn.unsubscribe(topic);
}

int MqttsnClientApplication::disconnect(uint16_t duration){
    return _mqttsn.disconnect(duration);
}

/*-------------- Callback related functions ---------------*/


void MqttsnClientApplication::refleshWdtCallbackTable(){
  _wdTimer.refleshRegisterTable();
}

/*======================================
               Class WdTimer
========================================*/
WdTimer::WdTimer(void) {
    _timerTbls = 0;
    _timerCnt = 0;
    _initFlg = true;

}

void WdTimer::start(void) {    
	//MQwatchdogEnable();
}


void WdTimer::stop(void){
    //
}


bool WdTimer::wakeUp(void){
    bool rcflg = false;
    int rc;
	for(uint8_t i = 0; i < _timerCnt; i++) {
		if ((_timerTbls[i].prevTime + _timerTbls[i].interval < (Clock_getMsCount()/1000)) || _initFlg){
			rc = (_timerTbls[i].callback)();
			if(rc == MQTTSN_ERR_REBOOT_REQUIRED){
				theApplication->initialize(theAppConfig);
			}
			_timerTbls[i].prevTime = Clock_getMsCount() / 1000;
			rcflg = true;
		}
	}
	_initFlg = false;
    return rcflg;
}

int WdTimer::registerCallback(uint32_t sec, int (*callback)(void)){
    MQ_TimerTbl *savTbl = _timerTbls;
    MQ_TimerTbl *newTbl = (MQ_TimerTbl*)calloc(_timerCnt + 1,sizeof(MQ_TimerTbl));

    if ( newTbl != 0 ) {
        _timerTbls = newTbl;
        for(uint8_t i = 0; i < _timerCnt; i++ ){
			_timerTbls[i].prevTime = savTbl[i].prevTime;
			_timerTbls[i].interval = savTbl[i].interval;
			_timerTbls[i].callback = savTbl[i].callback;
        }
        free(savTbl);

        _timerTbls[_timerCnt].prevTime = Clock_getMsCount() / 1000;
        _timerTbls[_timerCnt].interval = sec;
        _timerTbls[_timerCnt].callback = callback;
        _timerCnt++;
        return MQTTSN_ERR_NO_ERROR;
    }
    return MQTTSN_ERR_OUT_OF_MEMORY;
} 

void WdTimer::refleshRegisterTable(){
    for(uint8_t i = 0; i < _timerCnt; i++) {
        _timerTbls[i].prevTime = Clock_getMsCount() / 1000;
    }
}


#endif
#endif



