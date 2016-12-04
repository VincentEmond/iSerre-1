/*
 * Clock.h
 *
 *  Created on: 1 nov. 2016
 *      Author: Julien
 */

#ifndef SOURCES_CLOCK_H_
#define SOURCES_CLOCK_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Clock_resetMsCounter();

void Clock_incrementMsCounter();

uint64_t Clock_getMsCount();

#ifdef __cplusplus
}
#endif

#endif /* SOURCES_CLOCK_H_ */
