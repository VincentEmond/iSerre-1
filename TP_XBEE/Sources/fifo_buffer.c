/*
 * fifo_buffer.c
 *
 *  Created on: Nov 11, 2016
 *      Author: cheik
 */

#include "fifo_buffer.h"
#include "Xbee_Driver.h"


void init_fifo(struct Fifo *fifo)
{
	fifo->begin = 0;
	fifo->end = 0;
	flush_fifo(fifo);
}

void flush_fifo(struct Fifo *fifo)
{
	fifo->begin = 0;
	fifo->end = 0;
}

int lire_fifo_buffer(struct Fifo *fifo_buf, uint8_t *buffer_lecture)
{
	if(fifo_buf->begin == fifo_buf->end)
	{
		// buffer empty
		*buffer_lecture = 0;
		return -1; //
	}
	else
	{
		// Get buffer element at current index
		*buffer_lecture = fifo_buf->data[fifo_buf->begin];

		fifo_buf->begin++; // Shift index

		if(fifo_buf->begin == FIFO_SIZE)
			fifo_buf->begin = 0;

		return 0;
	}
}



