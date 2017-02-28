/*
 * iSN-Serveur.h
 *
 *  Created on: 2017/02/27
 *      Author: Vincent
 */

#ifndef SOURCES_ISN_H_
#define SOURCES_ISN_H_

#include "mqttsn/zbeeStack.h"
#include <stdint.h>
#include "stdio.h"

//Les messages ISN
#define ISN_MSG_SEARCH_SINK 		0x01
#define ISN_MSG_SEARCH_SINK_ACK 	0x05
#define ISN_MSG_CONFIG				0x02
#define ISN_MSG_COMMAND				0x03
#define ISN_MSG_MEASURE				0x04

//Les types de matériel ISN
#define ISN_SENSOR_TEMP				0x01
#define ISN_SENSOR_HUMI				0x02
#define ISN_ACTI_TEMP				0x81
#define ISN_ACTI_HUMI				0x82

/*Pour différentier le protocole iSN du protocole MQTT-SN
  Une trame qui commence par 0x02 en MQTT veux dire 2 octets de longueur.
  L'octet qui suit est le type de message et 0x03 est reservé dans MQTT donc on
  peut reconnaitre que c'est un message iSN et non pas un message MQTT-SN */
#define ISN_MAGIC_NUMBER			0x0203

//Le nombre maximal de clients pouvant se connecter au sink.
#define ISN_SINK_MAX_CLIENT				10


uint8_t MSB16(uint16_t x);
uint8_t LSB16(uint16_t x);

class IsnMessage
{
public:
	IsnMessage();
	~IsnMessage();
	uint8_t*& getPayload();
	uint16_t getLength();
	void printPayload();
protected:
	uint8_t* _buffer;
	uint16_t _length;
};

class IsnMsgSearchSink : public IsnMessage
{
public:
	IsnMsgSearchSink(uint8_t device_type);
private:
	uint8_t _deviceType;
};

class IsnMsgSearchSinkAck : public IsnMessage
{
public:
	IsnMsgSearchSinkAck(tomyClient::NWAddress64& sink_address);
private:
	tomyClient::NWAddress64& _sinkAddress;
};

class IsnMsgConfig : public IsnMessage
{

};

class IsnMsgCommand : public IsnMessage
{

};

class IsnMsgMeasure : public IsnMessage
{

};

class IsnServer
{
public:
	IsnServer();
	~IsnServer();

};

class IsnClient
{
public:
	IsnClient();
	~IsnClient();
	void executeProtocol();
};



void iSN_Start();
template<typename T> void writeDataToBuffer(T* data, uint8_t* buffer, int startIndex);



#endif /* SOURCES_ISN_H_ */
