/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_nand.c
 *    Description : lpc313x Nand driver
 *
 *    History :
 *    1. Date        : 03.4.2009 ã. 
 *       Author      : Stoyan Choynev
 *       Description : Initial Revision
*    2. Date         : 10.01.2010
 *
 *    $Revision: 30870 $
 **************************************************************************/
/** include files **/
#include "drv_cgu.h"
#include "drv_nand.h"

#include "nand.h"

/** local definitions **/
// NAND Flash device commands
#define NAND_RST                    0xFF
#define NAND_READ1                  0x00
#define NAND_READ2                  0x30
#define NAND_READ_ID                0x90
#define NAND_READ_STAT              0x70

#define NAND_PGM_PAGE1              0x80
#define NAND_PGM_PAGE2              0x10

#define NAND_BLOCK_ERASE1           0x60
#define NAND_BLOCK_ERASE2           0xD0

// NAND Flash status register
#define NAND_WRITE_PROT             0x80
#define NAND_ACTIVE                 0x40
#define NAND_ERROR                  0x01
/**/


#define NAND_SPARE_OFFSET     0x200     /**/
/** default settings **/

/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/

/** public functions **/
void InitNAND(void)
{
   /*Select EBI/MPMC pins*/
  SYSCREG_MUX_LCD_EBI_SEL_bit.Mux_LCD_EBI_sel = 1;
  CGU_Run_Clock(EBI_CLK);
  /*Enable MPMC clocks*/
  CGU_Run_Clock(MPMC_CFG_CLK);  
  CGU_Run_Clock(MPMC_CFG_CLK2);
  CGU_Run_Clock(MPMC_CFG_CLK3);

  /* Enable NAND Flash Controller Clocks */
  CGU_Run_Clock(NANDFLASH_S0_CLK);  
  CGU_Run_Clock(NANDFLASH_ECC_CLK);
  CGU_Run_Clock(NANDFLASH_NAND_CLK);  
  CGU_Run_Clock(NANDFLASH_PCLK);
  /*Reset NAND Flash Controller*/
  NANDFLASH_CTRL_ECC_RESET_N_SOFT = 0;
  NANDFLASH_CTRL_NAND_RESET_N_SOFT = 0;
  for(volatile Int32U i = 0 ; 10000 > i; i++);
  NANDFLASH_CTRL_ECC_RESET_N_SOFT = 1;
  NANDFLASH_CTRL_NAND_RESET_N_SOFT = 1;
  /**/
  SYSCREG_MUX_NAND_MCI_SEL_bit.Mux_NAND_MCI_sel = 0;
  
  /*Nand Configuration Register*/
  NandConfig = (1<<0)|     /* Error Correction On    */
               (0<<1)|     /* 8-bit flash device     */
               (0<<3)|     /* DMA disabled            */
               (0<<4)|     /* Little endian          */
               (3<<5)|     /* Latency configuration  */
               (1<<7)|     /* ECC started after 512B */
               (1<<8)|     /* CE deactivated         */
               (0<<10)|    /* 528 Bytes Read/Write   */
               (0<<12);    /* 5-bit mode ECC         */
                      
  NandSetCE = 0x1E;
  /* NAND Reset */
  /*Clear Status Flags*/
  NandIRQStatusRaw = 0xffffffff;    
  /*Erase Command*/
  NandSetCmd = NAND_RST;
  // wait for device ready
  while (!NandIRQStatusRaw_bit.INT28R); 
}

/*************************************************************************
 * Function name: NandGetID
 * Parameters:  None
 *
 * Return: None
 *
 * Description: Reads Chip ID in to the RBA 0
 *
 *************************************************************************/
unsigned int NandGetID(void)
{
unsigned int id = 0;
  /*Read ID Command*/
  NandSetCmd = NAND_READ_ID;
  /*Set Address*/
  NandSetAddr = 0x0;
  /*Read data Operation*/
  for(int i = 0; i < 4;i++ )
  {
    id = (id << 8) | (NandReadData & 0x000000FF);
  }
  return id;
}
/*************************************************************************
 * Function name: AddrInOperation
 * Parameters:  unsigned int Page
 *
 * Return: None
 *
 * Description: Perform Address Operation
 *
 *************************************************************************/
void AddrInOperation(unsigned int Column, unsigned int Row)
{
  /*Column*/
  for(int i = 0 ; 2 > i ; i++)
  {
    /*Load Column*/
    NandSetAddr = Column & 0xFF;
    /*Shift Column*/
     Column>>=8;
  }
  /*Row*/
  for(int i = 0 ; 3 > i ; i++)
  {
    /*Load Row*/
    NandSetAddr = Row & 0xFF;
    /*Shift Row*/
     Row>>=8;
  }
}

/*************************************************************************
 * Function name: NandGetStatus
 * Parameters:  None
 *
 * Return: None
 *
 * Description: Reads NAND Flash Status in to the RBA 0
 *
 *************************************************************************/
unsigned char NandGetStatus(void)
{
  /*Read Status Command*/
  NandSetCmd = NAND_READ_STAT;
  /**/
  for(volatile int delay  = 0; delay < 100; delay++);
  /*Read Data*/
  return NandReadData;
}
/*************************************************************************
 * Function Name: NandReadPage
 * Parameters: Page - NAND Flash Page Number. It is Block*NAND_PG_PER_BLK + Page_in_Block
 *             Buffer_Index - Index to NFC Buffer (0-3)
 * Return: FLASH_OK - No Read Error or 1bit Error
 *         FLASH_ERROR - Non Correcable Read Error
 * Description: Reads one page (512 main+16 spera) from NAND Flash into RBA
 *
 *************************************************************************/
unsigned int NandReadPage(unsigned char * dest, unsigned int Page,unsigned int Buff_Index)
{
    /*Read commad*/
    NandSetCmd = 0x00;
    /*Set Address*/
    AddrInOperation(528*(Page%4),Page/4);
    /*Clear Status Flags*/
    NandIRQStatusRaw = 0xffffffff;    
    /*Read Command*/
    NandSetCmd = 0x30;
    // wait for device ready
    while (!NandIRQStatusRaw_bit.INT28R);
    /*Clear Status flags*/
    NandIRQStatusRaw = 0xffffffff;
    /*Start Reading*/
    NandControlFlow = 1;
    /*Wait reading and ECC to complete*/
    while(!NandIRQStatusRaw_bit.INT21R);
    /*Non Correctable error*/
    if( !NandIRQStatusRaw_bit.INT26R && NandIRQStatusRaw_bit.INT11R) return FLASH_ERROR;

    unsigned char * src = (unsigned char *) 0x70000000;
    
    for(int i = 0 ; i<516; i+=sizeof(unsigned char))
    {
      *dest++ = *src++;
    }
    
    return FLASH_OK;
}
/*************************************************************************
 * Function Name: NandEraseBlock
 * Parameters: Block - NAND Block Number.
 *
 * Return: FLASH_OK - Erase Successful
 *         FLASH_ERROR - Erase Fail
 * Description: Erase One Block
 *
 *************************************************************************/
Int32U NandEraseBlock(Int32U Block)
{

unsigned int RowAddr = Block*(NAND_PG_PER_BLK/4);

  if(NAND_BLK_NUM <= Block) return FLASH_ERROR;
  /*Auto Block Erase Setup Command*/
  NandSetCmd = NAND_BLOCK_ERASE1;
  /*Row Address*/
  for(int i = 0 ; 3 > i ; i++)
  {
    /*Load Row*/
    NandSetAddr = RowAddr & 0xFF;
    /*Shift Row*/
    RowAddr>>=8;
  }  
  /*Clear Status*/
  NandIRQStatusRaw = 0xffffffff;    
  /*Erase Command*/
  NandSetCmd = NAND_BLOCK_ERASE2;
  // wait for device ready
  while (!NandIRQStatusRaw_bit.INT28R);
  /**/
  if(NandGetStatus() & 0x01)  return FLASH_ERROR;

  return FLASH_OK;
}

/*************************************************************************
 * Function Name: NandCheckBlock
 * Parameters: Block - NAND Block Number.
 *
 * Return: FLASH_OK -Valid Block
 *         FLASH_ERROR - Invalid Block
 * Description: Checks The Invalid Bytes in the Block
 *
 *************************************************************************/
unsigned int NandCheckBlock(unsigned char * dest, unsigned int Block)
{
unsigned int Page;
  /**/
  for(int cntr = 0; 2 > cntr; cntr++)
  {
    Page = cntr*4+3; /**/
    NandReadPage(dest, Block*NAND_PG_PER_BLK+Page,0);
    if(0xFF != (*((volatile unsigned int * )(FLASH_BASE_ADDR+464))&0xFF)) return FLASH_ERROR;
  }
  return FLASH_OK;
}
/*************************************************************************
 * Function Name: NandWritePage
 * Parameters: Page - NAND Flash Page Number. In is Block*NAND_PG_PER_BLK + Page_in_Block
 *             Buffer_Index - Index to NFC Buffer (0-3)
 * Return: FLASH_OK - Write Successful
 *         FLASH_ERROR - Write Fail
 * Description: Writes one page (512 main+16 spera) from RBA
 *              into NAND Flash
 *************************************************************************/
unsigned int NandWritePage(unsigned char * src,unsigned int Page, unsigned int Buff_Index)
{
unsigned char * dest = (unsigned char *) 0x70000000;
    /*Clear Status Flags*/
    NandIRQStatusRaw = 0xffffffff;    
    
    for(int i = 0 ; i<516; i+=sizeof(unsigned char))
    {
      *dest++ = *src++;
    }
    /*Serial Data input command*/
    NandSetCmd = NAND_PGM_PAGE1;
    /*Set Address*/
    AddrInOperation(528*(Page%4),Page/4);
    /*Wait ECC end*/
    while(!NandIRQStatusRaw_bit.INT20R);
    /*Start data transfer*/    
    NandControlFlow = 0x10;
    /*Wait trensfer end*/
    while(!NandIRQStatusRaw_bit.INT24R);
    /*Clear Status Flags*/
    NandIRQStatusRaw = 0xffffffff;    
    /*Erase Command*/
    NandSetCmd = NAND_PGM_PAGE2;
    // wait for device ready
    while (!NandIRQStatusRaw_bit.INT28R);
 
    if(NandGetStatus() & 0x01) return FLASH_ERROR;

   return FLASH_OK;
}

/**********/

unsigned int nand_EraseBlock(unsigned int block_num)
{
  //Auto Block Erase Setup Command
  NandSetCmd = NAND_BLOCK_ERASE1;
  
  // Page Address 1st 
      NandSetAddr = ((block_num << 6) & 0xC0);
  // Page Address 2nd 
      NandSetAddr = ((block_num << 6) >> 8) & 0xFF; // _BITMASK(8);
  // Page Address 3rd 
      NandSetAddr = ((block_num << 6) >> 16) & 0xFF; // _BITMASK(8);      
  //Clear Status
      NandIRQStatusRaw = 0xffffffff;    
  //Erase Command
      NandSetCmd = NAND_BLOCK_ERASE2;
  // wait for device ready
  while (!NandIRQStatusRaw_bit.INT28R);
 
  if(NandGetStatus() & 0x01)  return FLASH_ERROR;

  return FLASH_OK;
}

void nand_SetAddress(unsigned int block_num, unsigned int page_num, unsigned int column)
{
    // Send Column address 0 
    NandSetAddr = (column & 0xFF);

    // Second column address 
    NandSetAddr = ((column >> 8) & 0xFF);

    // Page address 0 
    NandSetAddr = (((page_num >> 0) & 0x003F) |
                       ((block_num << 6) & 0x00C0));

    // Page address 1 
    NandSetAddr = ((block_num >> 2) & 0x00FF);
 
    // Page address 2 
    NandSetAddr = ((block_num >> 10) & 0x0003);  
}

unsigned int nand_WritePage (unsigned char * src, unsigned int block_num, unsigned int page_num)
{
  unsigned char * dest = (unsigned char *) 0x70000000;
  
     
    //Serial Data input command
    NandSetCmd = NAND_PGM_PAGE1;
    //Set Address
    nand_SetAddress(block_num, page_num, 0);
    
    for (int k = 0; k<SECTORS_IN_PAGE; k++)
    {
      //Clear Status Flags
    NandIRQStatusRaw = 0xffffffff;       
    
      for(int i = 0 ; i<512; i++)
    {
      dest[i] = src[512*k+i];
    }
    
    //Wait ECC end
    while(!NandIRQStatusRaw_bit.INT20R);
    
    //Clear Status Flags
    NandIRQStatusRaw = 0xffffffff;       
    //Start data transfer
    NandControlFlow = 0x10;
    //Wait transfer end
    while(!NandIRQStatusRaw_bit.INT24R);
      
    }    
    
    //Clear Status Flags
    NandIRQStatusRaw = 0xffffffff;    
    //Program Command
    NandSetCmd = NAND_PGM_PAGE2;
    // wait for device ready
    while (!NandIRQStatusRaw_bit.INT28R);
    
    if(NandGetStatus() & 0x01) return FLASH_ERROR;

   return FLASH_OK;  
}

unsigned int nand_ReadPage (unsigned char * dest, unsigned int block_num, unsigned int page_num)
{  
  //Read command
  NandSetCmd = 0x00;
  //Set Address
  nand_SetAddress(block_num, page_num, 0);
  //Clear Status Flags
  NandIRQStatusRaw = 0xffffffff;  
  //Read Command
  NandSetCmd = 0x30;
  // wait for device ready
  while (!NandIRQStatusRaw_bit.INT28R);    
  
  for (int k = 0; k < SECTORS_IN_PAGE; k++)
  {
     //Clear Status flags
     NandIRQStatusRaw = 0xFFFFFFFF; 
     //Start Reading
     NandControlFlow = 1;       
     //Wait reading and ECC to complete
     while(!NandIRQStatusRaw_bit.INT21R);
     //Non Correctable error
     if( !NandIRQStatusRaw_bit.INT26R && NandIRQStatusRaw_bit.INT11R) return FLASH_ERROR;
     
    unsigned char * src = (unsigned char *) 0x70000000;    
    for(int i = 0 ; i<512; i++)
    {
      dest[k*512+i] = src[i];
    }    
    //We don't read 16 spare bytes from SRAM buffer        
  } 
  
  return FLASH_OK;
}

unsigned int nand_CheckBlock(unsigned char * dest, unsigned int Block)
{
/*unsigned int Page;
  
  for(int cntr = 0; 2 > cntr; cntr++)
  {
    Page = cntr*4+3; 
    NandReadPage(dest, Block*NAND_PG_PER_BLK+Page,0);
    if(0xFF != (*((volatile unsigned int * )(FLASH_BASE_ADDR+464))&0xFF)) return FLASH_ERROR;
  }
  */
  unsigned char * src = (unsigned char *) 0x70000000;
  nand_ReadPage(dest, Block, 0);    
  if (src[512] != 0xFF) return FLASH_ERROR;
  nand_ReadPage(dest, Block, 1);    
  if (src[512] != 0xFF) return FLASH_ERROR;
  
  return FLASH_OK;
}



/** private functions **/
