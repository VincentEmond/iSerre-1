/*
 * conversion.cpp
 *
 *  Created on: 2017-03-14
 *      Author: iSerre
 */

#include "conversion.h"
#include "string.h"

float bytesToFloat(unsigned char* buffer)
{
	float f;
	unsigned char b[] = {buffer[3], buffer[2], buffer[1], buffer[0]};
	memcpy(&f, &b, sizeof(f));
	return f;
}




