/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2005
 *
 *    File name   : audio_class.h
 *    Description : AUDIO CLASS definitions
 *
 *    History :
 *    1. Date        : November 29, 2005
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: #2 $
 **************************************************************************/
#include "includes.h"

#ifndef __AUDIO_CLASS_H
#define __AUDIO_CLASS_H

#ifdef AUDIO_CLASS_GLOBAL
#define AUDIO_CLASS_EXTERN
#else
#define AUDIO_CLASS_EXTERN  extern
#endif

#define MinVol        0x8000

#define Feat1MinVol   0xD000UL
#define Feat1MaxVol   0x0600UL
#define Feat1ResVol   0x0001UL

#define Feat2MinVol   0xD000UL
#define Feat2MaxVol   0x0600UL
#define Feat2ResVol   0x0001UL

typedef enum
{
  FeatUnit1Id = 1,
  SpkInTermID, SpkOutTermID,
  SelUnit1ID,
} AudioID_t;

extern Int32U SYS_GetFpclk(Int32U Periphery);

/*************************************************************************
 * Function Name: UsbAudioClassInit
 * Parameters: none
 *
 * Return: none
 *
 * Description: USB Class Audio Init
 *
 *************************************************************************/
void UsbAudioClassInit (void);

/*************************************************************************
 * Function Name: UsbClassAudioConfigure
 * Parameters:  pUsbDevCtrl_t pDev
 *
 * Return: none
 *
 * Description: USB Class Audio configure
 *
 *************************************************************************/
void UsbClassAudioConfigure (pUsbDevCtrl_t pDev);

/*************************************************************************
 * Function Name: AudioInHadler
 * Parameters:  USB_Endpoint_t EP
 *
 * Return: none
 *
 * Description: USB Class Audio Out EP handler
 *
 *************************************************************************/
static
void AudioInHadler (USB_Endpoint_t EP);

/*************************************************************************
 * Function Name: AudioFeatureGetReg
 * Parameters:  Int32U CS, Int32U Id
 *
 * Return: Boolean
 *
 * Description:
 *
 *************************************************************************/
static Boolean AudioFeatureGetReg (Int32U CS, Int32U Id);

/*************************************************************************
 * Function Name: UsbClassAudioRequest
 * Parameters:  pUsbSetupPacket_t pSetup
 *
 * Return: UsbCommStatus_t
 *
 * Description: USB Class Audio Requests
 *
 *************************************************************************/
UsbCommStatus_t UsbClassAudioRequest (pUsbSetupPacket_t pSetup);

/*************************************************************************
 * Function Name: UsbClassAudioData
 * Parameters:  USB_Endpoint_t EP
 *
 * Return: none
 *
 * Description: USB Class Audio Data receive
 *
 *************************************************************************/
static
void UsbClassAudioData (USB_Endpoint_t EP);

/*************************************************************************
 * Function Name: UsbAudioGetLinBuffer
 * Parameters: *pInt8U pStream
 *
 * Return: none
 *
 * Description: Return Linear buffer pointer and size
 *
 *************************************************************************/
Int32U UsbAudioGetLinBuffer(Int8U ** ppStream);

/*************************************************************************
 * Function Name: UsbAudioVolumeGetVolume
 * Parameters:  pInt32U pVol
 *
 * Return: Boolean
 *
 * Description: Return Volume value and update status
 *
 *************************************************************************/
Boolean UsbAudioVolumeGetVolume(pInt32U pVol);

/*************************************************************************
 * Function Name: UsbAudioGetPlayStatus
 * Parameters:  none
 *
 * Return: Boolean
 *
 * Description: Return current status (play/stop)
 *
 *************************************************************************/
Boolean UsbAudioGetPlayStatus(void);

#endif //__AUDIO_CLASS_H
