/*
 * fifo_buffer.h
 *
 *  Created on: Nov 11, 2016
 *      Author: cheik
 */

#ifndef SOURCES_FIFO_BUFFER_H_
#define SOURCES_FIFO_BUFFER_H_

#include "stdint.h"

#define FIFO_SIZE 128

struct Fifo
{
	uint8_t data[FIFO_SIZE];
	int begin;
	int end;
};

void init_fifo(struct Fifo *fifo);
int lire_fifo_buffer(struct Fifo *fifo_ptr, uint8_t *buffer_lecture);
void flush_fifo(struct Fifo *fifo);

#endif /* SOURCES_FIFO_BUFFER_H_ */
