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
#include "SDCard/lpc313x_mci_driver.h"
#include "drv_nand.h"
//#include "SDCard/lpc_types.h"
//#include "SDCard/lpc313x_chip.h"  

/*EFSL*/
#include "interfaces/lpc3131_mci_nand.h"
#include "config.h"

#include "nand.h"

unsigned char page0[PAGE_SIZE];
static unsigned long _nbad; //число bad blocks в диапазоне

#define NUMBER_OF_MODULES 11 //main program + 8 modules + 2 reserved blocks
int SoundImageOffset=1;
unsigned char SoundImageOffset_back;
int BadBlockListSize;
int BadBlocks[BAD_BLOCKS_LIST_SIZE];



int LoadBadBlockList ()
{
  int err; 
  
  err = nand_ReadPage(page0, 0, 1);  
  if (err == FLASH_ERROR) return 1; 
  
  BadBlockListSize = *(int*)page0;  
  
  for (int i=1; i <= BadBlockListSize; i++)
  {
    if((i-1)<BAD_BLOCKS_LIST_SIZE) //patch
      BadBlocks[i-1] = *(int*)(page0+i*sizeof(int)); 
  }
  
  return 0;   
}

int IfBlockBad(int BlockNumber) 
{
#if 0  
//MYDEBUG    
    if((BlockNumber==12) ||
       (BlockNumber==14) ||
       (BlockNumber==16) ||
       (BlockNumber==18) ||
       (BlockNumber==25) 
       )
      return 1;  
#endif
    
  for (int i = 0; i < BAD_BLOCKS_LIST_SIZE/*BadBlockListSize*/; i++) //patch
  {
    if (BlockNumber == BadBlocks[i])
      return 1; 
  }
  
  return 0; 
}

//найти число плохих блоков в диапазоне
void _nbad_blocks(unsigned long _begin, unsigned long _end)
{

  _nbad=0;  
  for (unsigned long i = _begin; i < _end; i++)  
  {
    if(IfBlockBad(i))
      _nbad++;
  }  
}

//вернуть число плохих блоков
unsigned long _get_nbad()
{
  return _nbad;
}





esint8 if_initInterface(hwInterface* file, eint8* opts)
{   
  UNS_32 blocknr; 
  MCI_CARD_INFO_T* psdcard ;
  
  if (*opts == SDCARD) 
  {  
        file->mcidev = mci_open((void*)SD_MMC_BASE, NULL);
        if (file->mcidev != NULL) 
        {
          psdcard = (MCI_CARD_INFO_T*)file->mcidev;        
          blocknr = psdcard->blocknr; //size[bytes] = blocknr * 512;                
   	  file->sectorCount = blocknr/512;
          file->DeviceType = SDCARD;
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
  
  
  if (*opts == NAND)
  {
    InitNAND();        
    LoadBadBlockList();    
    
  for (int i=1; i <= NUMBER_OF_MODULES; i++)
  {
    while (IfBlockBad(SoundImageOffset)) 
      SoundImageOffset++;         
    SoundImageOffset++;
  }
    //SoundImageOffset = 10;
    SoundImageOffset_back = SoundImageOffset;
    SoundImageOffset = SoundImageOffset*PAGES_IN_BLOCK*SECTORS_IN_PAGE;  //convert blocks to 512Byte-sectors
      
    
    //if (nand_ReadPage(page0, 0,0)== FLASH_ERROR) return -1 ;
    file->sectorCount = 1897*NAND_SECTORS_COUNT_COEFF;
    file->DeviceType = NAND;    
    return 0; 
  }  
  return -1; 
}

int GetValidBlockNumber(unsigned long _mod)
{
  int block=1; 
  LoadBadBlockList();
  for (int i=1; i <= _mod; i++)
  {
    while (IfBlockBad(block)) 
        block++;         
    block++;
  }
  
  return block-1; 
}

int GetParameterBlockNumber()
{
  int block=1; 
  LoadBadBlockList();
  for (int i=1; i <= NUMBER_OF_MODULES-2; i++)
  {
    while (IfBlockBad(block)) 
        block++;         
    block++;
  }
  
  return block; 
}

int GetTmpBlockNumber()
{  
  int block=1; 
  LoadBadBlockList();
  for (int i=1; i <= NUMBER_OF_MODULES-1; i++)
  {
    while (IfBlockBad(block)) 
        block++;         
    block++;
  }  
  return block;   
}


esint8 if_closeSD(hwInterface* file)
{
  if (file->DeviceType==SDCARD)
        mci_close(file->mcidev);
  return 0; 
}
/*****************************************************************************/

esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf){
        
	if (file->DeviceType==SDCARD)
              return(sd_readSector(file,address,buf,512));
        if (file->DeviceType==NAND)
              return nand_readSector(file,address,buf);          
        
}
/*****************************************************************************/

esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf)
{
        if (file->DeviceType==SDCARD)  
      	    return(sd_writeSector(file,address, buf));
         if (file->DeviceType==NAND) return 0;
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

esint8 nand_readSector(hwInterface *iface,euint32 address, euint8* buf)
{
  unsigned int block_num, page_num, sector_num; 
  int err; 
  unsigned long _i;
  
  address = address + SoundImageOffset;
  
  page_num = address / SECTORS_IN_PAGE;
  sector_num = address % SECTORS_IN_PAGE;
  block_num = page_num / PAGES_IN_BLOCK; 
  page_num = page_num%PAGES_IN_BLOCK;
  
  //LoadBadBlockList();
#if 0  
   while (IfBlockBad(block_num)) 
        block_num++; 
#else  
   _nbad_blocks(SoundImageOffset_back, block_num);
    
   for(_i=0;_i<_get_nbad();_i++)
   {
      while (IfBlockBad(block_num)) 
        block_num++;
      
      block_num++;
   }
  
   while (IfBlockBad(block_num)) 
        block_num++;   
#endif   
   
//-------------   
  
  err = nand_ReadPage(page0, block_num, page_num);
  
  if (err==FLASH_ERROR) return -1; 
  
  for (int i=0; i<512; i++)
    buf[i] = page0[512*sector_num+i];
  
  return 0;  
}



  
  

/*static int Card_Presence(void)
{
  IOCONF_EBI_MCI_M0_CLR = (1<<31);
  IOCONF_EBI_MCI_M1_CLR = (1<<31);
  for(volatile int i = 0; i<5000; i++);

  if(IOCONF_EBI_MCI_PIN & (1<<31)) return 1;
  return 0;
}*/