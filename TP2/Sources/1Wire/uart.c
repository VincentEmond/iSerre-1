/////////////////////////////////////////////////////////////////////////
///	\file uart.c
///	\brief Portable serial layer. 
///
///	\section UartLayer Serial Layer
///
///	Author: Ronald Alexander Nobrega De Sousa (Opticalworm)
///	Website: www.HashDefineElectronics.com
///
///	Licences:
///
///		Copyright (c) 2014 Ronald Alexander Nobrega De Sousa
///
///		Permission is hereby granted, free of charge, to any person obtaining a copy
///		of this software and associated documentation files (the "Software"), to deal
///		in the Software without restriction, including without limitation the rights
///		to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///		copies of the Software, and to permit persons to whom the Software is
///		furnished to do so, subject to the following conditions:
///
///		The above copyright notice and this permission notice shall be included in
///		all copies or substantial portions of the Software.
///	
///		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
///		THE SOFTWARE.
/////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "common.h"
#include "Uart1.h"

#define ONE_WIRE_UART_IDX 	Uart1_IDX
#define ONE_WIRE_UART 		UART1

/////////////////////////////////////////////////////////////////////////
///	\brief	setup the uart hardware
///
///	\param baud the desire baudrate
/////////////////////////////////////////////////////////////////////////
void Uart_Init(uint32_t baud)
{
	///	\todo Write code here setup the uart.
	uart_user_config_t uartConfig;
	uartConfig.baudRate = baud;
	uartConfig.bitCountPerChar = kUart8BitsPerChar;
	uartConfig.parityMode = kUartParityDisabled;
	uartConfig.stopBitCount = kUartOneStopBit;
	uart_state_t uartState;
	//UART_DRV_Init(ONE_WIRE_UART_IDX, &uartState, &uartConfig);

	//UART_HAL_SetReceiverSource(ONE_WIRE_UART, kUartSingleWire);
}

/////////////////////////////////////////////////////////////////////////
///	\brief	Set Uart baudrate. can be called at any time.
///
///	\param baud the desire baudrate
///
///	\note setting baudrate will effect any data currently been sent.
///		make sure that you check that the write buffer is empty
/////////////////////////////////////////////////////////////////////////
void Uart_Setbaud(uint32_t baud)
{
	///	\todo Write code here set the uart buadrate. 
	//UART_DRV_Deinit(ONE_WIRE_UART_IDX);
	Uart_Init(baud);
}

/////////////////////////////////////////////////////////////////////////
///	\brief	you can use this function to check if the write buffer is
///	empty and ready for new data
///
///	\param destination pointer to return the read byte
///	\return TRUE = empty and ready. else false
/////////////////////////////////////////////////////////////////////////
uint_fast8_t Uart_WriteBusy(void)
{
	uint_fast8_t ReturnState = TRUE;
	
	///	\todo Write code here that will get the MCU current write 
	/// buffer empty state and set ReturnState.
	
	return ReturnState;
}

/////////////////////////////////////////////////////////////////////////
///	\brief	Uart write single byte
///
///	\param source byte to write
/////////////////////////////////////////////////////////////////////////
void Uart_WriteByte(uint8_t source)
{
	///	\todo Write code here that transmit one byte.
	UART_HAL_SetTransmitterDir(ONE_WIRE_UART, kUartSinglewireTxdirOut);
	//UART_DRV_SendData(ONE_WIRE_UART_IDX, &source, 1);
	uart_status_t status = UART_DRV_SendDataBlocking(ONE_WIRE_UART_IDX, &source, 1, 1);
	if(status != kStatus_UART_Success)
		PRINTF("write : %d\n\r", status);
}

/////////////////////////////////////////////////////////////////////////
///	\brief	Uart read single byte
///
///	\param destination pointer to return the read byte
///	\return FALSE on success else TRUE
/////////////////////////////////////////////////////////////////////////
uint_fast8_t Uart_ReadByte(uint8_t * destination)
{
	uint_fast8_t ReturnState = TRUE;

	///	\todo Write code here that reads one byte from uart and 
	///	set ReturnState TRUE of false depending
	///	if it was sucessful.
	
	UART_HAL_SetTransmitterDir(ONE_WIRE_UART, kUartSinglewireTxdirIn);

	if(UART_DRV_ReceiveDataBlocking(ONE_WIRE_UART_IDX, destination, 1, 1) == kStatus_UART_Success)
		ReturnState = TRUE;
	else
		ReturnState = FALSE;

	return ReturnState;
}
