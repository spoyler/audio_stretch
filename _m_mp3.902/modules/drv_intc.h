/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_intc.h
 *    Description :  lpc313x Interrupt Controller Driver header file
 *
 *    History :
 *    1. Date        : 10.4.2009
 *       Author      : Stoyan Choynev
 *       Description : Initial Revison
 *
 *    $Revision: 30870 $
 **************************************************************************/
#ifndef __DRV_INTC_H
#define __DRV_INTC_H

/** include files **/

/** definitions **/

/** default settings **/

/** public data **/

/** public functions **/
__arm __irq void IRQ_Handler(void);
void INTC_Init(Int32U * VectorAddress);
void INTC_IRQInstall(VoidFpnt_t ISR, Int32U IRQIndex,
                     Int32U Priority, Int32U Active);
void INTC_IntEnable(Int32U IRQIndex, Int32U Enable);
#endif /* __DRV_INTC_H */