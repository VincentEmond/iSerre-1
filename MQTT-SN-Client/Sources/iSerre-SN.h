/*
 * iSerre-SN.h
 *
 *  Created on: 9 déc. 2016
 *      Author: Julien
 */

#ifndef SOURCES_ISERRE_SN_H_
#define SOURCES_ISERRE_SN_H_

#include <stdint.h>

#define ISN_CONFIG_COUNT_MAX (8u)


/*
 *
 */
typedef enum
{
	iSN_FrameType_SearchSink 	= 0x01,
	iSN_FrameType_SearchAck 		= 0x02,
	iSN_FrameType_Command 		= 0x03,
	iSN_FrameType_Measure 		= 0x04

} iSN_FrameType;


/*
 *
 */
typedef enum
{
	iSN_DeviceType_SensorTemperature 	= 0x01,
	iSN_DeviceType_SensorHumidity	 	= 0x02,

	iSN_DeviceType_ActuatorTemperature 	= 0x81,
	iSN_DeviceType_ActuatorHumidity 	= 0x82

} iSN_DeviceType;


typedef enum
{
	// Sensor/actuator statuses
	iSN_Status_SearchingSink = 0x1,
	iSN_Status_Sleeping = 0x2,

} iSN_Status;


/*
 *
 */

/*
 *
 */
typedef struct
{
	uint8_t type;
	uint8_t value;
} iSN_FrameSearchAck;

/*
 *
 */
typedef struct
{
	uint8_t frameType;
	uint8_t deviceType;

} iSN_FrameSearchSink;


/*
 *
 */
typedef struct
{
	uint8_t frameType;
	uint8_t command;

} iSN_FrameCommand;


/*
 *
 */
typedef struct
{
	uint8_t frameType;
	uint8_t measure[4];

} iSN_FrameMeasure;


#ifdef __cplusplus
extern "C" {
#endif

void convertFloat2Buffer(const float f, uint8_t* buffer)
{
	for(int i = 0 ; i < sizeof(float) ; i++)
		buffer[i] = ((uint8_t*)&pi)[i];
}

void convertBuffer2Float(const uint8_t* buffer, float* f)
{
	*f = *((float*)buffer);
}

#ifdef __cplusplus
}
#endif


#endif /* SOURCES_ISERRE_SN_H_ */
