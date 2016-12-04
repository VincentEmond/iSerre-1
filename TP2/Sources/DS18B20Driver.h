/*
 * DS18B20Driver.h
 *
 *  Created on: 7 oct. 2016
 *      Author: Julien
 */

#ifndef SOURCES_DS18B20DRIVER_H_
#define SOURCES_DS18B20DRIVER_H_

#include "stdint.h"

#define DS18B20_CMD_READ_ROM  			0x33
#define DS18B20_CMD_MATCH_ROM 			0x55
#define DS18B20_CMD_SEARCH_ROM 			0xF0
#define DS18B20_CMD_ALARM_SEARCH 		0xEC
#define DS18B20_CMD_SKIP_ROM 			0xCC
#define DS18B20_CMD_CONVERT_T 			0x44
#define DS18B20_CMD_READ_SCRATCHPAD 	0xBE
#define DS18B20_CMD_WRITE_SCRATCHPAD 	0x4E
#define DS18B20_CMD_COPY_SCRATCHPAD 	0x48
#define DS18B20_CMD_RECALL_E2 			0xB8
#define DS18B20_CMD_READ_POWER_SUPPLY 	0xB4

#define DS18B20_SCRATCHPAD_SIZE			(9U)

typedef struct DS18B20_scratchpad_s
{
	uint8_t data[DS18B20_SCRATCHPAD_SIZE];

} DS18B20_scratchpad_t;

uint16_t DS18B20_readTemperature();

void DS18B20_readScratchpad(DS18B20_scratchpad_t* scratchpad);

#endif /* SOURCES_DS18B20DRIVER_H_ */
