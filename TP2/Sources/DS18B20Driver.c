/*
 * DS18B20Driver.c
 *
 *  Created on: 26 oct. 2016
 *      Author: Julien
 */

#include "DS18B20Driver.h"
#include "1Wire/onewire.h"
#include "Console.h"

uint16_t DS18B20_readTemperature()
{
	DS18B20_scratchpad_t scratchpad;
	uint16_t temperature = 0;

	if(OneWire_Reset() == 1)
		PRINTF("fail\n\r");
	OneWire_Write(DS18B20_CMD_SKIP_ROM);
	OneWire_Write(DS18B20_CMD_CONVERT_T);

	DS18B20_readScratchpad(&scratchpad);

	temperature = (scratchpad.data[1] << 8) | scratchpad.data[0];

	return temperature;
}

void DS18B20_readScratchpad(DS18B20_scratchpad_t* scratchpad)
{
	if(OneWire_Reset() == 1)
		PRINTF("fail\n\r");
	OneWire_Write(DS18B20_CMD_SKIP_ROM);
	OneWire_Write(DS18B20_CMD_READ_SCRATCHPAD);

	for(int i = 0 ; i < DS18B20_SCRATCHPAD_SIZE ; i++)
		if(OneWire_Read(&scratchpad->data[i]) == 1)
			PRINTF("fail_temp\n\r");
}
