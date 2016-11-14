/* ###################################################################
**     Filename    : Events.c
**     Project     : Xbee Sandbox
**     Processor   : MKL26Z128VLH4
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2016-11-03, 11:48, # CodeGen: 0
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file Events.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         
/* MODULE Events */

#include "Cpu.h"
#include "Events.h"

#ifdef __cplusplus
extern "C" {
#endif 


/* User includes (#include below this line is not maintained by Processor Expert) */
#include "app.h"

/*
** ===================================================================
**     Callback    : uart_xb_RxCallback
**     Description : This callback occurs when data are received.
**     Parameters  :
**       instance - The UART instance number.
**       uartState - A pointer to the UART driver state structure
**       memory.
**     Returns : Nothing
** ===================================================================
*/
void uart_xb_RxCallback(uint32_t instance, void * uartState)
{
  /* Write your code here ... */
	uart_state_t *state = (uart_state_t*)uartState;
	struct Fifo* fifo = (struct Fifo*)(state->rxCallbackParam);

	fifo->data[fifo->end] = state->rxBuff[0];
	fifo->end++;
	if(fifo->end == FIFO_SIZE)
		fifo->end = 0;

}

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
