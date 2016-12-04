/*
 * Terminal.h
 *
 *  Created on: 4 oct. 2016
 *      Author: Julien
 */

#ifndef SOURCES_TERMINAL_H_
#define SOURCES_TERMINAL_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//void Terminal_ReadChar_NonBlocking(char* c);

//bool Terminal_KeyPressed();

void Terminal_ReadChar(char* value, bool echo);

void Terminal_SendChar(char value);

void Terminal_SendStr(char* str);

void Terminal_Clear();

void Terminal_CRLF();

void Terminal_CursorHome();

void Terminal_SendNum(int number, int base);

uint32_t Terminal_ReadUInt32(bool echo);

uint32_t Terminal_ReadStr(uint8_t *str, size_t maxByteCount, bool echo);

#ifdef __cplusplus
}
#endif

#endif /* SOURCES_TERMINAL_H_ */
