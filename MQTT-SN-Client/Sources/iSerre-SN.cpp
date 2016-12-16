#include "iSerre-SN.h"
#include "stdlib.h"

void convertFloat2Buffer(const float f, uint8_t* buffer)
{
	for(size_t i = 0 ; i < sizeof(float) ; i++)
		buffer[i] = ((uint8_t*)&f)[i];
}

void convertBuffer2Float(const uint8_t* buffer, float* f)
{
	*f = *((float*)buffer);
}
