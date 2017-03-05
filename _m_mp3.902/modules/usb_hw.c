/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    File name   : usb_hw.c
 *    Description : usb module (HAL)
 *
 *    History :
 *    1. Date        : August 7, 2009
 *       Author      : Stanimir Bonev
 *       Description : Create
 *        USB framework rev. 2 light version
 *
 *    $Revision: 31493 $
 **************************************************************************/
#define USB_HW_GLOBAL
#include "usb_hw.h"

typedef union _EP_Cap_t
{
  Int32U Data;
  struct {
    Int32U                    : 15;
    Int32U IOS                : 1;
    Int32U Max_packet_length  : 11;
    Int32U                    : 2;
    Int32U ZLT                : 1;
    Int32U MULT               : 2;
  };
} EP_Cap_t;

typedef struct _dTD_t
{
  Int32U  dTDNext;
  struct {
    Int32U Status       : 8;
    Int32U              : 2;
    Int32U MultO        : 2;
    Int32U              : 3;
    Int32U IOC          : 1;
    Int32U Total_bytes  :13;
    Int32U              : 1;
  };
  Int32U BufferPage[5];
  Int32U Reserved;
} dTD_t, * pdTD_t;

typedef struct _dQH_t
{
  EP_Cap_t          EP_Cap;
  Int32U            dTD_Curr;
  dTD_t             dTD;
  UsbSetupPacket_t  Setup;
  Int32U            Reserved1[4];
} dQH_t, *pdQH_t;

#pragma data_alignment=2048
static volatile dQH_t dQH[ENP_MAX_NUMB];

#pragma data_alignment=32
static volatile dTD_t dTD[ENP_MAX_NUMB];

static const UsbStandardEpDescriptor_t USB_CtrlEpDescr0 =
{
  sizeof(UsbStandardEpDescriptor_t),
  UsbDescriptorEp,
  UsbEpOut(CTRL_ENP_OUT>>1),
  {(Int8U)UsbEpTransferControl | (Int8U)UsbEpSynchNoSynchronization | (Int8U)UsbEpUsageData},
  Ep0MaxSize,
  0
};

static const UsbEP_ExtData_t USB_CtrlEpExtDescr0 =
{
  0
};

static const UsbStandardEpDescriptor_t USB_CtrlEpDescr1 =
{
  sizeof(UsbStandardEpDescriptor_t),
  UsbDescriptorEp,
  UsbEpIn(CTRL_ENP_IN>>1),
  {(Int8U)UsbEpTransferControl | (Int8U)UsbEpSynchNoSynchronization | (Int8U)UsbEpUsageData},
  Ep0MaxSize,
  0
};

static const UsbEP_ExtData_t USB_CtrlEpExtDescr1 =
{
  1,0
};

/*************************************************************************
 * Function Name: USB_HwInit
 * Parameters: none
 *
 * Return: none
 *
 * Description: Init USB
 *
 *************************************************************************/
void USB_HwInit(void)
{
  // Turn on USB Clk
  CGU_Run_Clock(USB_OTG_AHB_CLK);

  // Enable PLL USB
  SYSCREG_USB_ATX_PLL_PD_REG = 0;
  while(!ER_PEND3_bit.usb_atx_pll_lock);

  // Reset OTG Controller
  OTG_USBCMD_REG_bit.RST = 1;
  while(OTG_USBCMD_REG_bit.RST);

  // Enable Device controller
  OTG_USBMODE_REG_bit.CM = 2;

#if USB_HIGH_SPEED > 0
  // FS or HS enabled
  OTG_PORTSC1_REG_bit.PFSC = 0;
#else
  // Fosce to FS device only
  OTG_PORTSC1_REG_bit.PFSC = 1;
#endif

  // Disable USB interrupts
  OTG_USBINTR_REG = 0;

  // Init Device queue heads
  OTG_ENDPOINTLISTADDR_REG = (Int32U)dQH;

  // clear all pending interrupts
  OTG_USBSTS_REG = 0x000101C7;

  // Disable All EP
  OTG_ENDPTCTRL1_REG_bit.RXE = 0;
  OTG_ENDPTCTRL1_REG_bit.TXE = 0;
  OTG_ENDPTCTRL2_REG_bit.RXE = 0;
  OTG_ENDPTCTRL2_REG_bit.TXE = 0;
  OTG_ENDPTCTRL3_REG_bit.RXE = 0;
  OTG_ENDPTCTRL3_REG_bit.TXE = 0;

  // Init controls endpoints
  USB_HwReset();

  // Init Device status
  UsbSetDevState(UsbDevStatusUnknow);

  // Enable USB interrupts
  // Install Interrupt Service Routine, Priority
  INTC_IRQInstall(USB_ISR, USB_OTG_IRQ,
                  USB_INTR_PRIORITY,0);

  // Enable USB interrupt
  INTC_IntEnable(USB_OTG_IRQ, 1);

}

/*************************************************************************
 * Function Name: USB_HwReset
 * Parameters: none
 *
 * Return: none
 *
 * Description: Reset USB engine
 *
 *************************************************************************/
void USB_HwReset (void)
{
  // Clear all pending setup tolken semaphores
  OTG_ENDPTSETUPSTAT_REG = OTG_ENDPTSETUPSTAT_REG;
  // Clear all EP complete status flags
  OTG_ENDPTCOMPLETE_REG = OTG_ENDPTCOMPLETE_REG;
  // Cancel all primed status
  while(OTG_ENDPTPRIME_REG);  // !!!
  OTG_ENDPTFLUSH_REG = -1UL;
  // Enable trip wire mechanism
  //OTG_USBMODE_REG_bit.SLOM = 1;
  OTG_USBMODE_REG_bit.SLOM = 0;

  // Init dQH
  for(Int32U i = 0; i < sizeof(dQH)/sizeof(dQH[1]); i++)
  {
    dQH[i].dTD_Curr   = 1;
    dQH[i].dTD.Status = 0;
  }

  for(Int32U i = 0; i < sizeof(EpCnfg)/sizeof(EpCnfg[1]); i++)
  {
    EpCnfg[i].Status  = NOT_VALID;
  }

  // Control EP Init
  USB_RealizeEp(&USB_CtrlEpDescr0,&USB_CtrlEpExtDescr0,TRUE);
  USB_RealizeEp(&USB_CtrlEpDescr1,&USB_CtrlEpExtDescr1,TRUE);

  // Enable Device interrupts
  OTG_USBINTR_REG = (1UL << 0)   // UE
                  | (1UL << 2)   // PCE
                  | (1UL << 6)   // URE
                  | (1UL << 8)   // SLE
                  | (USB_SOF_EVENT   ? (1UL << 7) : 0)  // SRE
                  | (USB_ERROR_EVENT ? (1UL << 1) : 0)  // UEE
                  ;

}


/*************************************************************************
 * Function Name: USB_FlushEP
 * Parameters: USB_Endpoint_t EP
 *
 * Return: none
 *
 * Description: Flush EP
 *
 *************************************************************************/
void USB_FlushEP (USB_Endpoint_t EP)
{
  if(USB_EP_VALID(EpCnfg + EP))
  {
    switch(EP)
    {
    case CTRL_ENP_OUT:
      while ((1<<0) & OTG_ENDPTPRIME_REG)
      {
        OTG_ENDPTFLUSH_REG = (1<<0);
      }
      break;
    case CTRL_ENP_IN:
      while ((1UL<<16) & OTG_ENDPTPRIME_REG)
      {
        OTG_ENDPTFLUSH_REG = (1UL<<16);
      }
      break;
    case ENP1_OUT:
      while ((1UL<<1) & OTG_ENDPTPRIME_REG)
      {
        OTG_ENDPTFLUSH_REG = (1UL<<1);
      }
      break;
    case ENP1_IN:
      while ((1UL<<17) & OTG_ENDPTPRIME_REG)
      {
        OTG_ENDPTFLUSH_REG = (1UL<<17);
      }
      break;
    case ENP2_OUT:
      while ((1UL<<2) & OTG_ENDPTPRIME_REG)
      {
        OTG_ENDPTFLUSH_REG = (1UL<<2);
      }
      break;
    case ENP2_IN:
      while ((1UL<<18) & OTG_ENDPTPRIME_REG)
      {
        OTG_ENDPTFLUSH_REG = (1UL<<18);
      }
      break;
    case ENP3_OUT:
      while ((1UL<<3) & OTG_ENDPTPRIME_REG)
      {
        OTG_ENDPTFLUSH_REG = (1UL<<3);
      }
      break;
    case ENP3_IN:
      while ((1UL<<19) & OTG_ENDPTPRIME_REG)
      {
        OTG_ENDPTFLUSH_REG = (1UL<<19);
      }
      break;
    }
    dQH[EP].dTD.dTDNext = 1;
    dQH[EP].dTD.Status  = 0;
  }
}

/*************************************************************************
 * Function Name: USB_RealizeEp
 * Parameters: const UsbStandardEpDescriptor_t * pEP_Desc,
 *             const UsbEP_ExtData_t * pUsbEP_ExtData,
 *             Boolean Enable
 *
 * Return: USB_ErrorCodes_t
 *
 * Description: Enable or disable an endpoint
 *
 *************************************************************************/
USB_ErrorCodes_t USB_RealizeEp(const UsbStandardEpDescriptor_t * pEP_Desc,
                               const UsbEP_ExtData_t * pUsbEP_ExtData,
                               Boolean Enable)
{
USB_Endpoint_t EP = (USB_Endpoint_t)USB_EpLogToPhysAdd(pEP_Desc->bEndpointAddress);

  if (Enable)
  {
    switch(EP)
    {
    case CTRL_ENP_OUT:
      dQH[EP].EP_Cap.IOS = 1;
    case CTRL_ENP_IN:
      break;
    case ENP1_OUT:
      OTG_ENDPTCTRL1_REG_bit.RXS = 0;
      OTG_ENDPTCTRL1_REG_bit.RXR = 1;
      OTG_ENDPTCTRL1_REG_bit.RXT = pEP_Desc->bmAttributes.TransferType;
      OTG_ENDPTCTRL1_REG_bit.RXE = 1;
      break;
    case ENP1_IN:
      OTG_ENDPTCTRL1_REG_bit.TXS = 0;
      OTG_ENDPTCTRL1_REG_bit.TXR = 1;
      OTG_ENDPTCTRL1_REG_bit.TXT = pEP_Desc->bmAttributes.TransferType;
      OTG_ENDPTCTRL1_REG_bit.TXE = 1;
      break;
    case ENP2_OUT:
      OTG_ENDPTCTRL2_REG_bit.RXS = 0;
      OTG_ENDPTCTRL2_REG_bit.RXR = 1;
      OTG_ENDPTCTRL2_REG_bit.RXT = pEP_Desc->bmAttributes.TransferType;
      OTG_ENDPTCTRL2_REG_bit.RXE = 1;
      break;
    case ENP2_IN:
      OTG_ENDPTCTRL2_REG_bit.TXS = 0;
      OTG_ENDPTCTRL2_REG_bit.TXR = 1;
      OTG_ENDPTCTRL2_REG_bit.TXT = pEP_Desc->bmAttributes.TransferType;
      OTG_ENDPTCTRL2_REG_bit.TXE = 1;
      break;
    case ENP3_OUT:
      OTG_ENDPTCTRL3_REG_bit.RXS = 0;
      OTG_ENDPTCTRL3_REG_bit.RXR = 1;
      OTG_ENDPTCTRL3_REG_bit.RXT = pEP_Desc->bmAttributes.TransferType;
      OTG_ENDPTCTRL3_REG_bit.RXE = 1;
      break;
    case ENP3_IN:
      OTG_ENDPTCTRL3_REG_bit.TXS = 0;
      OTG_ENDPTCTRL3_REG_bit.TXR = 1;
      OTG_ENDPTCTRL3_REG_bit.TXT = pEP_Desc->bmAttributes.TransferType;
      OTG_ENDPTCTRL3_REG_bit.TXE = 1;
      break;
    default:
      return(USB_EP_NOT_VALID);
    }

    // Init dQH
    dQH[EP].EP_Cap.Max_packet_length = pEP_Desc->wMaxPacketSize;
    dQH[EP].EP_Cap.ZLT  = pUsbEP_ExtData->ZLT;
    dQH[EP].EP_Cap.MULT = pUsbEP_ExtData->MULT;
    dQH[EP].dTD_Curr   |= 1;
    dQH[EP].dTD.Status  = 0;
    // Set EP status
    EpCnfg[EP].Status   = NOT_READY;

  }
  else
  {
    // Flush and Disable relevant endpoint
    USB_FlushEP(EP);
    switch(EP)
    {
    case ENP1_OUT:
      OTG_ENDPTCTRL1_REG_bit.RXE = 0;
      break;
    case ENP1_IN:
      OTG_ENDPTCTRL1_REG_bit.TXE = 0;
      break;
    case ENP2_OUT:
      OTG_ENDPTCTRL2_REG_bit.RXE = 0;
      break;
    case ENP2_IN:
      OTG_ENDPTCTRL2_REG_bit.TXE = 0;
      break;
    case ENP3_OUT:
      OTG_ENDPTCTRL3_REG_bit.RXE = 0;
      break;
    case ENP3_IN:
      OTG_ENDPTCTRL3_REG_bit.TXE = 0;
      break;
    }
    EpCnfg[EP].Status = NOT_VALID;
  }
  return(USB_OK);
}

/*************************************************************************
 * Function Name: USB_SetAdd
 * Parameters: Int32U DevAdd - device address between 0 - 127
 *
 * Return: none
 *
 * Description: Set device address
 *
 *************************************************************************/
void USB_SetAdd(Int32U DevAdd)
{
  OTG_DEVICEADDR_REG  = (DevAdd << 25);
  OTG_DEVICEADDR_REG |= (1UL << 24);
}

/*************************************************************************
 * Function Name: USB_ConnectRes
 * Parameters: Boolean Conn
 *
 * Return: none
 *
 * Description: Connect USB
 *
 *************************************************************************/
void USB_ConnectRes (Boolean Conn)
{
  OTG_USBCMD_REG_bit.RS = (0 != Conn);
}

/*************************************************************************
 * Function Name: USB_Configure
 * Parameters: Boolean Configure
 *
 * Return: none
 *
 * Description: Configure device
 *              When Configure != 0 enable all Realize Ep
 *
 *************************************************************************/
void USB_Configure (Boolean Configure)
{
}

#if USB_REMOTE_WAKEUP != 0
/*************************************************************************
 * Function Name: USB_WakeUp
 * Parameters: none
 *
 * Return: none
 *
 * Description: Wake up USB
 *
 *************************************************************************/
void USB_WakeUp (void)
{
  OTG_PORTSC1_REG_bit.FPR = 1;
  UsbDevSuspendCallback(0);
}
#endif // USB_REMOTE_WAKEUP != 0

/*************************************************************************
 * Function Name: USB_GetDevStatus
 * Parameters: USB_DevStatusReqType_t Type
 *
 * Return: Boolean
 *
 * Description: Return USB device status
 *
 *************************************************************************/
Boolean USB_GetDevStatus (USB_DevStatusReqType_t Type)
{
  switch (Type)
  {
  case USB_DevConnectStatus:
    return(OTG_PORTSC1_REG_bit.CCS);
  case USB_SuspendStatus:
    return(OTG_PORTSC1_REG_bit.SUSP);
  case USB_ResetStatus:
    return(OTG_PORTSC1_REG_bit.PR);
  case USB_HighSpeedStatus:
    return(OTG_PORTSC1_REG_bit.HSP);
  }
  return(FALSE);
}

/*************************************************************************
 * Function Name: USB_SetStallEP
 * Parameters: USB_Endpoint_t EndPoint, Boolean Stall
 *
 * Return: none
 *
 * Description: The endpoint stall/unstall
 *
 *************************************************************************/
void USB_SetStallEP (USB_Endpoint_t EP, Boolean Stall)
{
  switch(EP)
  {
  case CTRL_ENP_OUT:
  case CTRL_ENP_IN:
    if(Stall)
    {
      OTG_ENDPTCTRL0_REG |= (1UL << 0) | (1UL << 16);
    }
    else
    {
      OTG_ENDPTCTRL0_REG &= ~((1UL << 0) | (1UL << 16));
    }
    break;
  case ENP1_OUT:
    if(Stall)
    {
      if(!OTG_ENDPTCTRL1_REG_bit.RXS)
      {
        OTG_ENDPTCTRL1_REG_bit.RXS = 1;
      }
    }
    else
    {
      if(OTG_ENDPTCTRL1_REG_bit.RXS)
      {
        OTG_ENDPTCTRL1_REG_bit.RXR = 1;
        OTG_ENDPTCTRL1_REG_bit.RXS = 0;
      }
    }
    break;
  case ENP1_IN:
    if(Stall)
    {
      if(!OTG_ENDPTCTRL1_REG_bit.TXS)
      {
        OTG_ENDPTCTRL1_REG_bit.TXS = 1;
      }
    }
    else
    {
      if(OTG_ENDPTCTRL1_REG_bit.TXS)
      {
        OTG_ENDPTCTRL1_REG_bit.TXR = 1;
        OTG_ENDPTCTRL1_REG_bit.TXS = 0;
      }
    }
    break;
  case ENP2_OUT:
    if(Stall)
    {
      if(!OTG_ENDPTCTRL2_REG_bit.RXS)
      {
        OTG_ENDPTCTRL2_REG_bit.RXS = 1;
      }
    }
    else
    {
      if(OTG_ENDPTCTRL2_REG_bit.RXS)
      {
        OTG_ENDPTCTRL2_REG_bit.RXR = 1;
        OTG_ENDPTCTRL2_REG_bit.RXS = 0;
      }
    }
    break;
  case ENP2_IN:
    if(Stall)
    {
      if(!OTG_ENDPTCTRL2_REG_bit.TXS)
      {
        OTG_ENDPTCTRL2_REG_bit.TXS = 1;
      }
    }
    else
    {
      if(OTG_ENDPTCTRL2_REG_bit.TXS)
      {
        OTG_ENDPTCTRL2_REG_bit.TXR = 1;
        OTG_ENDPTCTRL2_REG_bit.TXS = 0;
      }
    }
    break;
  case ENP3_OUT:
    if(Stall)
    {
      if(!OTG_ENDPTCTRL3_REG_bit.RXS)
      {
        OTG_ENDPTCTRL3_REG_bit.RXS = 1;
      }
    }
    else
    {
      if(OTG_ENDPTCTRL3_REG_bit.RXS)
      {
        OTG_ENDPTCTRL3_REG_bit.RXR = 1;
        OTG_ENDPTCTRL3_REG_bit.RXS = 0;
      }
    }
    break;
  case ENP3_IN:
    if(Stall)
    {
      if(!OTG_ENDPTCTRL3_REG_bit.TXS)
      {
        OTG_ENDPTCTRL3_REG_bit.TXS = 1;
      }
    }
    else
    {
      if(OTG_ENDPTCTRL3_REG_bit.TXS)
      {
        OTG_ENDPTCTRL3_REG_bit.TXR = 1;
        OTG_ENDPTCTRL3_REG_bit.TXS = 0;
      }
    }
    break;
  default:
    return;
  }
  EpCnfg[EP].Status = Stall?STALLED:NOT_READY;
}

/*************************************************************************
 * Function Name: USB_StallCtrlEP
 * Parameters: none
 *
 * Return: none
 *
 * Description: Stall both direction of the CTRL EP
 *
 *************************************************************************/
void USB_StallCtrlEP (void)
{
  USB_SetStallEP(CTRL_ENP_OUT,TRUE);
}

/*************************************************************************
 * Function Name: USB_GetStallEP
 * Parameters: USB_Endpoint_t EP, pBoolean pStall
 *
 * Return: none
 *
 * Description: Get stall state of the endpoint
 *
 *************************************************************************/
void USB_GetStallEP (USB_Endpoint_t EP, pBoolean pStall)
{
  switch(EP)
  {
  case CTRL_ENP_OUT:
    *pStall = OTG_ENDPTCTRL0_REG_bit.RXS;
    break;
  case CTRL_ENP_IN:
    *pStall = OTG_ENDPTCTRL0_REG_bit.TXS;
    break;
  case ENP1_OUT:
    *pStall = OTG_ENDPTCTRL1_REG_bit.RXS;
    break;
  case ENP1_IN:
    *pStall = OTG_ENDPTCTRL1_REG_bit.TXS;
    break;
  case ENP2_OUT:
    *pStall = OTG_ENDPTCTRL2_REG_bit.RXS;
    break;
  case ENP2_IN:
    *pStall = OTG_ENDPTCTRL2_REG_bit.TXS;
    break;
  case ENP3_OUT:
    *pStall = OTG_ENDPTCTRL3_REG_bit.RXS;
    break;
  case ENP3_IN:
    *pStall = OTG_ENDPTCTRL3_REG_bit.TXS;
    break;
  }
}

/*************************************************************************
 * Function Name: USB_EP_IO
 * Parameters: USB_Endpoint_t EndPoint
 *
 * Return: none
 *
 * Description: Endpoint Write (IN)
 *
 *************************************************************************/
void USB_EP_IO(USB_Endpoint_t EP)
{
volatile dTD_t * p_dtd;
pEpCnfg_t p_ep;
Int32U size;

  p_ep                 = &EpCnfg[EP];
  p_ep->Status         =  BEGIN_SERVICED;
  p_dtd                = &dTD[EP];
  memset((void *)p_dtd,0,sizeof(dTD[1]));
  p_dtd->dTDNext      |=  1;
  size                 =  p_ep->Size;
  p_dtd->Total_bytes   =  size;
  p_dtd->BufferPage[0] = (Int32U)p_ep->pBuffer;
  p_dtd->BufferPage[1] = (p_dtd->BufferPage[0] + 4096UL) & ~(4096UL-1UL) ;
  p_dtd->BufferPage[2] =  p_dtd->BufferPage[1] + 4096UL;
  p_dtd->BufferPage[3] =  p_dtd->BufferPage[2] + 4096UL;
  p_dtd->BufferPage[4] =  p_dtd->BufferPage[3] + 4096UL;
  p_dtd->IOC           =  1;
  p_dtd->Status        = (1UL<<7);
  dQH[EP].dTD.Status   =  0;
  dQH[EP].dTD.dTDNext  = (Int32U)p_dtd;

  switch(EP)
  {
  case CTRL_ENP_OUT:
    OTG_ENDPTPRIME_REG = 1UL << 0;
    break;
  case CTRL_ENP_IN:
    OTG_ENDPTPRIME_REG = 1UL << 16;
    break;
  case ENP1_OUT:
    OTG_ENDPTPRIME_REG = 1UL << 1;
    break;
  case ENP1_IN:
    OTG_ENDPTPRIME_REG = 1UL << 17;
    break;
  case ENP2_OUT:
    OTG_ENDPTPRIME_REG = 1UL << 2;
    break;
  case ENP2_IN:
    OTG_ENDPTPRIME_REG = 1UL << 18;
    break;
  case ENP3_OUT:
    OTG_ENDPTPRIME_REG = 1UL << 3;
    break;
  case ENP3_IN:
    OTG_ENDPTPRIME_REG = 1UL << 19;
    break;
  }
}

/*************************************************************************
 * Function Name: USB_EpLogToPhysAdd
 * Parameters: Int8U EpLogAdd
 *
 * Return: USB_Endpoint_t
 *
 * Description: Convert the logical to physical address
 *
 *************************************************************************/
USB_Endpoint_t USB_EpLogToPhysAdd (Int8U EpLogAdd)
{
USB_Endpoint_t Address = (USB_Endpoint_t)((EpLogAdd & 0x0F)<<1);
  if(EpLogAdd & 0x80)
  {
    ++Address;
  }
  return(Address);
}

#if USB_SOF_EVENT > 0
/*************************************************************************
 * Function Name: USB_GetFrameNumb
 * Parameters: none
 *
 * Return: Int32U
 *
 * Description: Return current value of SOF number
 *
 *************************************************************************/
Int32U USB_GetFrameNumb (void)
{
  return(OTG_FRINDEX_REG);
}
#endif

/*************************************************************************
 * Function Name: USB_StatusPhase
 * Parameters: Boolean In
 *
 * Return: none
 *
 * Description: Prepare status phase
 *
 *************************************************************************/
void USB_StatusPhase (Boolean In)
{
  if(In)
  {
    USB_IO_Data(CTRL_ENP_IN,NULL,0,NULL);
  }
  else
  {
    USB_IO_Data(CTRL_ENP_OUT,NULL,0,NULL);
  }
}

/*************************************************************************
 * Function Name: USB_ISR
 * Parameters: none
 *
 * Return: none
 *
 * Description: USB interrupt subroutine
 *
 *************************************************************************/
void USB_ISR (void)
{
Int32U intr;

  USB_INTR_ENTRY_HOOK();

  intr  = OTG_USBSTS_REG;
  intr &= OTG_USBINTR_REG;

  if((1UL << 0) & intr) // EP Interrupts
  {
    OTG_USBSTS_REG = (1UL << 0);
    // Check for Setup Packet
    if(1 & OTG_ENDPTSETUPSTAT_REG)
    {
      do
      {
        USB_FlushEP(CTRL_ENP_OUT);
        USB_IO_Data(CTRL_ENP_OUT,NULL,(Int32U)-1,NULL);
        USB_FlushEP(CTRL_ENP_IN);
        USB_IO_Data(CTRL_ENP_IN,NULL,(Int32U)-1,NULL);
        do
        {
          do
          {
            OTG_ENDPTSETUPSTAT_REG = 1;
          }
          while(1 & OTG_ENDPTSETUPSTAT_REG);
          OTG_USBCMD_REG_bit.SUTW = 1;
          memcpy(UsbEp0SetupPacket.Data,(void *)dQH[0].Setup.Data,sizeof(UsbEp0SetupPacket));
        }
        while(0 == OTG_USBCMD_REG_bit.SUTW);

        OTG_USBCMD_REG_bit.SUTW = 0;

        if (UsbEp0SetupPacket.mRequestType.Dir == UsbDevice2Host)
        {
          USB_StatusHandler(CTRL_ENP_OUT);
        }

        USB_SetupHandler();
      }
      while(1 & OTG_ENDPTSETUPSTAT_REG);

      if(EpCnfg[CTRL_ENP_OUT].Status == STALLED)
      {
        USB_StallCtrlEP();
      }
    }

    if(0xFUL & OTG_ENDPTCOMPLETE_REG) // Check for TC
    {
      for(Int32U mask = 1, ep = 0; mask <= 1<<3; mask <<= 1, ep += 2)
      {
        if(mask & OTG_ENDPTCOMPLETE_REG)
        {
          OTG_ENDPTCOMPLETE_REG = mask;
          // check transfer status
          if(0 == dTD[ep].Status)
          {
            EpCnfg[ep].Size -= dTD[ep].Total_bytes;
            if(dTD[ep].Total_bytes)
            {
              EpCnfg[ep].Status = BUFFER_UNDERRUN;
            }
            else
            {
              EpCnfg[ep].Status = COMPLETE;
            }
          }
          else
          {
            EpCnfg[ep].Status = BUFFER_OVERRUN;
          }

          if(EpCnfg[ep].pFn)
          {
            ((void(*)(USB_Endpoint_t))EpCnfg[ep].pFn)((USB_Endpoint_t)ep);
          }
        }
      }
    }
    if((0xFUL << 16)& OTG_ENDPTCOMPLETE_REG) // Check for TC
    {
      for(Int32U mask = 1UL<<16, ep = 1; mask <= 1UL<<19; mask <<= 1, ep+=2)
      {
        if(mask & OTG_ENDPTCOMPLETE_REG)
        {
          OTG_ENDPTCOMPLETE_REG = mask;
          EpCnfg[ep].Size -= dTD[ep].Total_bytes;
          // check transfer status
          if(0 == dTD[ep].Status)
          {
            EpCnfg[ep].Status = COMPLETE;
          }
          else
          {
            EpCnfg[ep].Status = BUFFER_OVERRUN;
          }

          if(EpCnfg[ep].pFn)
          {
            ((void(*)(USB_Endpoint_t))EpCnfg[ep].pFn)((USB_Endpoint_t)ep);
          }
        }
      }
    }
  }

  if((1UL << 7) & intr) // SOF
  {
    OTG_USBSTS_REG = (1UL << 7);
  #if USB_SOF_EVENT > 0
    USB_FRAME_HOOK(USB_GetFrameNumb());
  #else
    USB_FRAME_HOOK(0);
  #endif
  }

  if((1UL << 2) & intr) // Port change
  {
    OTG_USBSTS_REG = (1UL << 2);
#if USB_HIGH_SPEED > 0
    UsbDevSpeedCallback(OTG_PORTSC1_REG_bit.HSP);
#endif // USB_HIGH_SPEED > 0
  }

  if((1UL << 8) & intr) // Suspend
  {
    OTG_USBSTS_REG = (1UL << 8);
    UsbDevSuspendCallback(OTG_PORTSC1_REG_bit.SUSP);
  }

  if((1UL << 6) & intr) // Reset
  {
    OTG_USBSTS_REG = (1UL << 6);
    USB_HwReset();
    UsbDevSuspendCallback(0);
    UsbDevResetCallback();
  }

#if USB_ERROR_EVENT > 0
  if((1UL << 1) & intr) // Error
  {
    OTG_USBSTS_REG = (1UL << 1);
    USB_ERR_HOOK(0);
  }
#endif

  USB_INTR_EXIT_HOOK();
}
