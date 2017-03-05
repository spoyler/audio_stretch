/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename :lpc3131_mci.c                                                    *
* Description : This file contains the functions needed to use efs for        *
*               accessing files on an SD-card connected to an LPC3131.        *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
                                                                              *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
\*****************************************************************************/
#include <stdio.h>
#include "SDCard/lpc313x_mci_driver.h"
//#include "SDCard/lpc_types.h"
//#include "SDCard/lpc313x_chip.h"  

/*EFSL*/
#include "interfaces/lpc3131_mci.h"
#include "config.h"

esint8 if_initInterface(hwInterface* file, eint8* opts)
{   
  UNS_32 blocknr; 
  MCI_CARD_INFO_T* psdcard ;
  
        file->mcidev = mci_open((void*)SD_MMC_BASE, NULL);
        if (file->mcidev != NULL) 
        {
          psdcard = (MCI_CARD_INFO_T*)file->mcidev;        
          blocknr = psdcard->blocknr; //size[bytes] = blocknr * 512;                
   	  file->sectorCount = blocknr/512;
	  if( (blocknr%512) != 0)   file->sectorCount--;
	
          DBG((TXT("Init done...\n")));
          return(0);
        }
        else return (-1);
        
       /* print card info */
       /* PRINTF( "%s card acquired\r\n", (psdcard->card_type & CARD_TYPE_SD) ? "SD" : "MMC");
        PRINTF( "rca 0x%x\r\n", psdcard->rca);
        PRINTF( "cid: 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
                psdcard->cid[0], psdcard->cid[1], psdcard->cid[2], psdcard->cid[3]);

        PRINTF( "csd: 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
                psdcard->csd[0], psdcard->csd[1], psdcard->csd[2], psdcard->csd[3]);

        PRINTF( "card size: %d.%02d MiB\r\n",psdcard->device_size / (1024*1024),
                (((psdcard->device_size / 1024) % 1024)*100) / 1024);

        if ( psdcard->ext_csd[53] != 0)
        {
          PRINTF("ext_csd:\r\n");
          i = 0;
          while (i < (MMC_SECTOR_SIZE/4))
          {
            PRINTF( "0x%08x 0x%08x 0x%08x 0x%08x\r\n",
                    psdcard->ext_csd[i], psdcard->ext_csd[i+1], psdcard->ext_csd[i+2], psdcard->ext_csd[i+3]);
            i += 4;
          }
        }*/       
        
        
 
}

esint8 if_close(hwInterface* file)
{
  mci_close(file->mcidev);
  return 0; 
}
/*****************************************************************************/

esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf){
        
	return(sd_readSector(file,address,buf,512));
}
/*****************************************************************************/

esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf)
{
          
	return(sd_writeSector(file,address, buf));
}
/*****************************************************************************/

esint8 if_setPos(hwInterface* file,euint32 address)
{
	return(0);
}

esint8 sd_readSector(hwInterface *iface,euint32 address, euint8* buf, euint16 len)
{
  mci_wait(iface->mcidev);
  if (mci_read_blocks(iface->mcidev, buf,address, address) <= 0)
	{
		return (-1);
	}
	
	return (0);
  
}

esint8 sd_writeSector(hwInterface *iface,euint32 address, euint8* buf)
{
  mci_wait(iface->mcidev);  
  if  (mci_write_blocks(iface->mcidev, buf, address, address) <= 0)
  {
    return (-1);
  }
  
  return (0);
  
}
  
  

/*static int Card_Presence(void)
{
  IOCONF_EBI_MCI_M0_CLR = (1<<31);
  IOCONF_EBI_MCI_M1_CLR = (1<<31);
  for(volatile int i = 0; i<5000; i++);

  if(IOCONF_EBI_MCI_PIN & (1<<31)) return 1;
  return 0;
}*/