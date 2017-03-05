/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : usb_cnfg.h
 *    Description : USB config file
 *
 *    History :
 *    1. Date        : June 16, 2007
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 30870 $
 **************************************************************************/

#include "includes.h"

#ifndef __USB_CNFG_H
#define __USB_CNFG_H

/* USB High Speed support*/
#define USB_HIGH_SPEED                  1

/* USB interrupt priority */
#define USB_INTR_PRIORITY               15
#define USB_DEV_PRIORITY                0   // 1 - Frame is high priority,
                                            // 2 - EPs are high priority
/* Endpoint priority setting*/
#define USB_EP_PRIORITY                 0x00000000

/* USB Events */
#define USB_SOF_EVENT                   0
#define USB_ERROR_EVENT                 1   // for debug
#define USB_SOF_FRAME_NUMB              0   // disable frame number

//DMA Settings
#define USB_DMA_DD_MAX_NUMB             1   // number of DMA descriptors
#define USB_DMA_ID_MAX_NUMB             1   // number of Isochronous DMA descriptors
#define DMA_INT_ENABLE_MASK             5   // DMA interrupt enable (End of Transfer,
                                            // New DD request, System error interrupt)
/* USB PORT settings */
#define USB_PORT_SEL                    2

/* USB Clock settings */
#define USB_CLK_DIV                     6

/* Device power atrb  */
#define USB_SELF_POWERED                0
#define USB_REMOTE_WAKEUP               1

/* Max Interfaces number*/
#define USB_MAX_INTERFACE               1

/* Endpoint definitions */
#define Ep0MaxSize                      64

#define SpkEp         				          ENP1_OUT
#define SpkEpMaxSize   				          (SampRerFrame * SubFrameSize)
#define SpkDDInd                        0

/* Class defenitions*/
#define USB_CTRL_INTERFACE              0
#define USB_SPK_INTERFACE               1

#define SubFrameSize                    4
#define SampRerFrame                    44
#define SampFreq                        (44100)

/* Other defenitions */

#endif //__USB_CNFG_H
