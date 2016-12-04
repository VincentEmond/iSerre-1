/*
 * Clock.c
 *
 *  Created on: 1 nov. 2016
 *      Author: Julien
 */

#include "Clock.h"

static volatile uint64_t Clock_currentMs = 0;

void Clock_resetMsCounter()
{
	Clock_currentMs = 0;
}

void Clock_incrementMsCounter()
{
	Clock_currentMs++;
}

uint64_t Clock_getMsCount()
{
	return Clock_currentMs;
}
