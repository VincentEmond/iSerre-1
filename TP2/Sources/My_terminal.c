/*
 * My_terminal.c
 *
 *  Created on: 4 oct. 2016
 *      Author: Julien
 */

#include "My_terminal.h"
#include "stdlib.h"
#include "string.h"

void Terminal_ReadChar_NonBlocking(char* c)
{
	if(Terminal_KeyPressed())
		Terminal_ReadChar(c);
}

uint32_t getuint32(size_t character_buffer_size)
{
	char buf[character_buffer_size];
	memset(buf,0,character_buffer_size);

	for(uint32_t i = 0 ; i < character_buffer_size ; i++)
	{
		char c = 0;
		Terminal_ReadChar(&c);

		if(i == 0 && (c < '0' || c > '9')) // skip trailing non numeric characters
		{
			i--;
			continue;
		}
		else if(i != 0 && (c < '0' || c > '9')) // break if one non numeric character is read
			break;

		buf[i] = c;
		Terminal_SendChar(c);
	}
	Terminal_CRLF();

	return (uint32_t)strtoul(buf, NULL, 0);
}


