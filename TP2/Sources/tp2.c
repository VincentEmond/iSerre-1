/*
 * tp2.c
 *
 *  Created on: 26 oct. 2016
 *      Author: Julien
 */

#include "tp2.h"
#include "Menu.h"
#include "My_terminal.h"
#include "1Wire/onewire.h"
#include "DS18B20Driver.h"

void read_sensor_temperature()
{
	OneWire_Reset();
	uint8_t rom[8] = {1,1,1,1,1,1,1,1};
	OneWire_Write(DS18B20_CMD_READ_ROM);
	for(int i = 0 ; i < 8 ; i++)
		OneWire_Read(&rom[i]);

	for(int i = 0 ; i < 8 ; i++)
		PRINTF("rom %d:%d ; ", i, (int)rom[i]);

	/*DS18B20_readTemperature();

	DS18B20_scratchpad_t s;
	DS18B20_readScratchpad(&s);

	for(int i = 0 ; i < DS18B20_SCRATCHPAD_SIZE ; i++)
		PRINTF("temperature : %d ", s.data[i]);
	PRINTF("\n\r");*/

	/*uint16_t temp = DS18B20_readTemperature();
	PRINTF("temp : %u\n\r", temp);*/
}

void tp2_run()
{
	PRINTF("coucou\n\r");

	OneWire_Init();

	Menu_init("TP2");
	Menu_add_item("readTemp", read_sensor_temperature);
	Menu_print();

	for(;;)
	{
		char userInput = 0;
		Terminal_ReadChar(&userInput);
		Menu_choose(userInput-'0');
	}

}


