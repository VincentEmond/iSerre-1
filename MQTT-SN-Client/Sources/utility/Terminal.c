/*
 * Terminal.c
 *
 *  Created on: 4 oct. 2016
 *      Author: Julien
 */

#include "Terminal.h"
#include "StringUtility.h"

#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

// Add user include here
#include "UART_Com0.h"

#define STRING_TO_U32_BUFFER_SIZE 20U

//void Terminal_ReadChar_NonBlocking(char* c)
//{
//	if(Terminal_KeyPressed())
//		Terminal_ReadChar(c, false);
//}
//
//
//bool Terminal_KeyPressed()
//{
//	// TODO : implement
//	return true;
//}

uint32_t Terminal_ReadUInt32(bool echo)
{
	char buf[STRING_TO_U32_BUFFER_SIZE];
	memset(buf,0,STRING_TO_U32_BUFFER_SIZE);

	for(uint32_t i = 0 ; i < STRING_TO_U32_BUFFER_SIZE ; i++)
	{
		char c = 0;
		Terminal_ReadChar(&c, false);

		if(i == 0 && (c < '0' || c > '9')) // skip trailing non numeric characters
		{
			i--;
			continue;
		}
		else if(i != 0 && (c < '0' || c > '9')) // break if one non numeric character is read
			break;

		buf[i] = c;
		if(echo)
			PUTCHAR(c);
	}
	Terminal_CRLF();

	return (uint32_t)strtoul(buf, NULL, 0);
}

void Terminal_ReadChar(char* value, bool echo)
{
	*value = GETCHAR();
	if(echo)
		PUTCHAR(*value);
}

void Terminal_SendChar(char value)
{
	PUTCHAR(value);
}

/*
 * Send a character string to Console output.
 * Use it instead of PRINTF if you only want to print string and not number
 */
void Terminal_SendStr(char* str)
{
	char* ptr = str;
	while(*ptr)
	{
		PUTCHAR(*ptr);
		ptr++;
	}
}

void Terminal_Clear()
{
	Terminal_SendStr("\x1B[2J");
}

void Terminal_CRLF()
{
	Terminal_SendStr("\n\r");
}

void Terminal_CursorHome()
{
	Terminal_SendStr("\x1B[H");
}

void Terminal_SendNum(int number, int base)
{
	char temp[20];
	itoa(number, temp, base);
	Terminal_SendStr(temp);
}

uint32_t Terminal_ReadStr(uint8_t *str, size_t size, bool echo)
{
	uint32_t byteReadedCount;
	for(byteReadedCount = 0 ; byteReadedCount < size ; byteReadedCount++)
	{
		char c = 0;
		c = GETCHAR();

		if(c == '\n' || c == '\r')
			break;

		str[byteReadedCount] = c;
		if(echo)
			PUTCHAR(c);
	}
	Terminal_CRLF("\n\r");

	return byteReadedCount;
}
