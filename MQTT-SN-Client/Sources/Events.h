/* ###################################################################
**     Filename    : Events.h
**     Project     : MQTT-SN
**     Processor   : MKL26Z128VLH4
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2016-11-01, 20:24, # CodeGen: 0
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file Events.h
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         

#ifndef __Events_H
#define __Events_H
/* MODULE Events */

#include "fsl_device_registers.h"
#include "clockMan1.h"
#include "pin_init.h"
#include "osa1.h"
#include "PIT_Timer0.h"
#include "UART_Com2.h"
#include "UART_Com0.h"

#ifdef __cplusplus
extern "C" {
#endif 

#ifdef PIT_Timer0_IDX
/*
** ===================================================================
**     Interrupt handler : PIT_IRQHandler
**
**     Description :
**         User interrupt service routine. 
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void PIT_IRQHandler(void);
#else
  /* This IRQ handler is not used by PIT_Timer0 component. The purpose may be
   * that the component has been removed or disabled. It is recommended to 
   * remove this handler because Processor Expert cannot modify it according to 
   * possible new request (e.g. in case that another component uses this
   * interrupt vector). */
  #warning This IRQ handler is not used by PIT_Timer0 component.\
           It is recommended to remove this because Processor Expert cannot\
           modify it according to possible new request.
#endif

/*
** ===================================================================
**     Callback    : UART_Com2_TxCallback
**     Description : This callback occurs when data are transmitted.
**     Parameters  :
**       instance - The UART instance number.
**       uartState - A pointer to the UART driver state structure
**       memory.
**     Returns : Nothing
** ===================================================================
*/
void UART_Com2_TxCallback(uint32_t instance, void * uartState);

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

#endif 
/* ifndef __Events_H*/
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
