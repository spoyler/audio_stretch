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
#include "lpc32xx_sdcard_driver.h"
//#include "SDCard/lpc_types.h"
//#include "SDCard/lpc313x_chip.h"  

/*EFSL*/
#include "interfaces/lpc3250_mmc.h"
#include "config.h"

#include "nand.h"
#include "lpc32xx_slcnand_driver.h"

#include "lpc32xx_dma_driver.h"
#include "lpc32xx_intc_driver.h"


unsigned char page0[PAGE_SIZE];
static unsigned long _nbad;

extern INT_32 nand;

extern NAND_GEOM_T geom;
extern UNS_8 _rdbuff[];

#define NUMBER_OF_MODULES 22 //main program + 8 modules + 2 reserved blocks
int SoundImageOffset=1;
unsigned char SoundImageOffset_back;
int BadBlockListSize;
int BadBlocks[BAD_BLOCKS_LIST_SIZE];



extern INT_32 sdmmc_read_block(UNS_32 *buff,
                        INT_32 numblks, /* Must be 1 */
                        UNS_32 index,
                        SD_CMDRESP_T *resp);
extern INT_32 sdmmc_write_block(UNS_32 *buff,
                        INT_32 numblks, /* Must be 1 */
                        UNS_32 index,
                        SD_CMDRESP_T *resp);

int LoadBadBlockList ()
{
  int err; 
//without bad block check  
#if 1  
 /* //_DEBUG
  err = nand_ReadPage(page0, 0, 1);  
  if (err == FLASH_ERROR) return 1; 
  */
//bad block list 63 page 0 block 
  err = slcnand_read_sector(nand, 63, _rdbuff, 0);
  if (err==_ERROR) return -1; 
  
  
  BadBlockListSize = *(int*)_rdbuff;  
  
  for (int i=1; i <= BadBlockListSize; i++)
  {
    if((i-1)<BAD_BLOCKS_LIST_SIZE) //patch
      BadBlocks[i-1] = *(int*)(_rdbuff+i*sizeof(int)); 
  }
#else
  BadBlockListSize = 0;
#endif  
  
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
    
  for (int i = 0; i < BadBlockListSize; i++)
  {
    if(i<BAD_BLOCKS_LIST_SIZE)
    if (BlockNumber == BadBlocks[i])
      return 1; 
  }
  
  return 0; 
}

//????? ????? ?????? ?????? ? ?????????
void _nbad_blocks(unsigned long _begin, unsigned long _end)
{

  _nbad=0;  
  for (unsigned long i = _begin; i < _end; i++)  
  {
    if(IfBlockBad(i))
      _nbad++;
  }  
}

//??????? ????? ?????? ??????
unsigned long _get_nbad()
{
  return _nbad;
}


esint8 if_initInterface(hwInterface* file, eint8* opts)
{ 

  if (*opts == _SDCARD) 
  {  
  
        file->DeviceType = _SDCARD;
  //UNS_32 blocknr; 
  //MCI_CARD_INFO_T* psdcard ;
  
        /*file->mcidev = mci_open((void*)SD_MMC_BASE, NULL);
        if (file->mcidev != NULL) 
        {
          psdcard = (MCI_CARD_INFO_T*)file->mcidev;        
          blocknr = psdcard->blocknr; //size[bytes] = blocknr * 512;                
   	  file->sectorCount = blocknr/512;
	  if( (blocknr%512) != 0)   file->sectorCount--;
	
          DBG((TXT("Init done...\n")));
          return(0);
        }
        else return (-1);*/
  return (0);
        
  }

  if (*opts == NAND)
  {
     
    LoadBadBlockList();    
    
  for (int i=1; i < NUMBER_OF_MODULES; i++)
  {
    while (IfBlockBad(SoundImageOffset)) 
      SoundImageOffset++;         
    SoundImageOffset++;
  }
    //SoundImageOffset = 10;
    SoundImageOffset_back = SoundImageOffset;
    SoundImageOffset = SoundImageOffset*PAGES_IN_BLOCK*SECTORS_IN_PAGE;  //convert blocks to 512Byte-sectors
      
    
    //if (nand_ReadPage(page0, 0,0)== FLASH_ERROR) return -1 ;
    //file->sectorCount = 1897*NAND_SECTORS_COUNT_COEFF;
    file->DeviceType = NAND;    
    return 0; 
  }  
  
       // print card info
       // PRINTF( "%s card acquired\r\n", (psdcard->card_type & CARD_TYPE_SD) ? "SD" : "MMC");
        //PRINTF( "rca 0x%x\r\n", psdcard->rca);
        //PRINTF( "cid: 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
        //        psdcard->cid[0], psdcard->cid[1], psdcard->cid[2], psdcard->cid[3]);

        //PRINTF( "csd: 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
        //        psdcard->csd[0], psdcard->csd[1], psdcard->csd[2], psdcard->csd[3]);

        //PRINTF( "card size: %d.%02d MiB\r\n",psdcard->device_size / (1024*1024),
        //        (((psdcard->device_size / 1024) % 1024)*100) / 1024);

        //if ( psdcard->ext_csd[53] != 0)
        //{
        //  PRINTF("ext_csd:\r\n");
        //  i = 0;
        //  while (i < (MMC_SECTOR_SIZE/4))
        //  {
        //    PRINTF( "0x%08x 0x%08x 0x%08x 0x%08x\r\n",
        //           psdcard->ext_csd[i], psdcard->ext_csd[i+1], psdcard->ext_csd[i+2], psdcard->ext_csd[i+3]);
        //   i += 4;
         // }
        //}       
        
        
 
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


esint8 if_close(hwInterface* file)
{
  //mci_close(file->mcidev);
  if (file->DeviceType==_SDCARD)
    sdmmc_close();
  if (file->DeviceType==NAND)
    slcnand_close(nand);
  
  return 0; 
}
/*****************************************************************************/

esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf){
        
  if (file->DeviceType==_SDCARD)
	return(sd_readSector(file,address,buf,512));
  if (file->DeviceType==NAND)
        return nand_readSector(file,address,buf);    
}
/*****************************************************************************/

esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf)
{
  if (file->DeviceType==_SDCARD)         
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
  UNS_32 *dbuff; 
  SD_CMDRESP_T resp;
  
  dbuff = (UNS_32 *) buf;
  
  int_disable(IRQ_DMA);
  //if (mci_read_blocks(iface->mcidev, buf,address, address) <= 0)
    if (sdmmc_read_block(dbuff, 1, address, &resp) < 0)
	{
                int_enable(IRQ_DMA);
		return (-1);
	}
	
       int_enable(IRQ_DMA);
	return (0);
  
}

esint8 sd_writeSector(hwInterface *iface,euint32 address, euint8* buf)
{
  
  UNS_32 *dbuff; 
  SD_CMDRESP_T resp;
  
  dbuff = (UNS_32 *) buf;
  
  //if  (mci_write_blocks(iface->mcidev, buf, address, address) <= 0)
  if (sdmmc_write_block(dbuff, 1, address, &resp) < 0)
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
  
//address -> 512 bytes adr  
  
  address = address + SoundImageOffset;
  
  page_num = address / SECTORS_IN_PAGE;
  sector_num = address % SECTORS_IN_PAGE;
  block_num = page_num / PAGES_IN_BLOCK; 
  //page_num = page_num%PAGES_IN_BLOCK;
  
  LoadBadBlockList();
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
  err = slcnand_read_sector(nand, page_num, _rdbuff, 0);
  if (err==_ERROR) return -1; 
   
  
  for (int i=0; i<512; i++)
    buf[i] = _rdbuff[512*sector_num+i];
    
  
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