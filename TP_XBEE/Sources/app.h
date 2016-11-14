/*
 * app.h
 *
 *  Created on: Nov 7, 2016
 *      Author: cheik
 */

#ifndef SOURCES_APP_H_
#define SOURCES_APP_H_

#include "DbgCs1.h"
#include "uart_xb.h"
#include "console.h"
#include "Xbee_Driver.h"
#include "fifo_buffer.h"

struct Fifo fifo_xbee;
uint8_t xb_rxBuff[1];

void app_run();

//void uart_xb_RxCallback


#endif /* SOURCES_APP_H_ */
