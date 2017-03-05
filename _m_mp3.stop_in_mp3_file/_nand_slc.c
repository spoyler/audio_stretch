/***********************************************************************
 * $Id:: kickstart_burner.c 8211 2011-10-05 17:00:11Z usb10132        $
 *
 * Project: MLC burner for kickstart or stage 1
 *
 * Description:
 *     This version programs the kickstart or stage 1 application using
 *     the MLC with large block support.
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#include <string.h>
#include "lpc_types.h"
#include "lpc_arm922t_cp15_driver.h"
#include "lpc32xx_dma_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_slcnand_driver.h"
#include "lpc_irq_fiq.h"
//#include "common_funcs.h"


#define MAX_PAGE_SIZE (2048 + 64)
/* NAND read and write buffers */
/*DMA_BUFFER*/ UNS_8 _rdbuff[MAX_PAGE_SIZE];
#if 0
static /*DMA_BUFFER*/ UNS_8 wrbuff[MAX_PAGE_SIZE];
#endif
static UNS_16 *wrbuff16;
static UNS_8 *rdbuff8, *wrbuff8;
INT_32 nand;
NAND_GEOM_T geom;


/***********************************************************************
 *
 * Function: isblksame
 *
 * Purpose: Verify 2 blocks are the same
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff1 : Pointer to buffer 1 to verify
 *     buff2 : Pointer to buffer 2 to verify
 *
 * Outputs: None
 *
 * Returns: -1 on pass, or failed index
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 isblksame(UNS_8 *buff1, UNS_8 *buff2)
{
	INT_32 idx;

    for (idx = 0; idx < geom.data_bytes_per_page; idx++)
    {
    	if (buff1[idx] != buff2[idx])
    	{
			return idx;
    	}
    }

    return -1;
}

/***********************************************************************
 *
 * Function: compdata
 *
 * Purpose: Verify an entire buffer has the same value
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff1 : Pointer to buffer 1 to verify
 *     data : value to verify against
 *
 * Outputs: None
 *
 * Returns: -1 on pass, or failed index
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 compdata(UNS_8 *buff1, UNS_8 data)
{
	INT_32 idx;

    for (idx = 0; idx < geom.data_bytes_per_page; idx++)
    {
    	if (buff1[idx] != data)
    	{
	   		return idx;
    	}
    }

    return -1;
}

/***********************************************************************
 *
 * Function: nand_write_sectors
 *
 * Purpose: Write data to a large block device
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector : Sector to write
 *     wrbuffer : Pointer to buffer to write
 *     size : Total number of bytes
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void nand_write_sectors(int sector, UNS_8 *wrbuffer, int size)
{
		while (size > 0)
		{
			slcnand_write_sector(nand, sector, wrbuffer, 0);
			wrbuffer = wrbuffer + geom.data_bytes_per_page;
			size = size - geom.data_bytes_per_page;
			sector++;
		}
}

/***********************************************************************
 *
 * Function: verify_data
 *
 * Purpose: Verify data written to a device
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector : Sector to verify
 *     rdbuffer : Pointer to buffer to verify
 *     size : Total number of bytes
 *
 * Outputs: None
 *
 * Returns: -1 on pass, or failed index
 *
 * Notes: None
 *
 **********************************************************************/
int verify_data(int sector, UNS_8 *wrbuffer, UNS_8 *rdbuffer, int size)
{
	int idx = 0;
		while (size > 0)
		{
			slcnand_read_sector(nand, sector, rdbuffer, 0);
			idx = isblksame(rdbuffer, wrbuffer);
			if (idx >= 0){
   				return idx;
   			}
 			wrbuffer = wrbuffer + geom.data_bytes_per_page;
   			size = size - geom.data_bytes_per_page;
   			sector++;
		}

		return -1;
}

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point from the startup code
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns 1, or <0 on an error
 *
 * Notes: None
 *
 **********************************************************************/
void _nand_setup(void)
{
  UNS_32 ptw, sector, progblk, loadsize;
  UNS_8 tmp16, tmp16i, *p8;
  int idx = 0;



  nand = slcnand_board_init();
  /* Init NAND controller */
  if (nand == 0){
	while (1)
          asm("nop");
  }

 
  slcnand_get_geom(nand, &geom);
#if 0 

  /* Disable write protect */
  //nand_flash_wp_disable();

  wrbuff16 = (UNS_16 *) wrbuff;
  rdbuff8 = (UNS_8 *) rdbuff;
  wrbuff8 = (UNS_8 *) wrbuff;

  /* Setup a write block with a bad block marker in case it is
     needed */
  memset(wrbuff, 0xFF, MAX_PAGE_SIZE);
  wrbuff8[geom.data_bytes_per_page + NAND_LB_BADBLOCK_OFFS] = (UNS_8) ~NAND_GOOD_BLOCK_MARKER;

  progblk = 0;

  sector = progblk * geom.pages_per_block;

  /* Get sector address for first page of block */
  if (slcnand_erase_block(nand, progblk) == _ERROR)
  {
	/* Erase failure, mark the block as bad */
   	slcnand_write_sector(nand, sector, wrbuff8, 0);
  }

  /* Really, Block is erased? */
  slcnand_read_sector(nand, sector, rdbuff8, 0);
  idx = compdata(rdbuff8, 0xFF);
  if(idx >= 0)
  {
		slcnand_write_sector(nand, sector, wrbuff8, 0);
		while (1);
  }


  /* Setup the write buffer for page #0 ICR data */
  memset(wrbuff, 0x00, MAX_PAGE_SIZE);

  /* Setup FLASH config See UM10326 section 2.2.3.2 */
  tmp16 = 0x00; /* 16-Bit NAND still not supported */
  if (geom.data_bytes_per_page == 512)
  {
    if (geom.address_cycles > 3)
    {
      tmp16 |= 1 << 1;
    }
  }
  else
  {
    tmp16 |= 1 << 2;
    if (geom.address_cycles > 4)
    {
      tmp16 |= 1 << 1;
    }
  }
  tmp16 |= (~tmp16 & 0x0F) << 4;
  tmp16i = 0xFF - tmp16;
  wrbuff16[0] = tmp16;
  wrbuff16[2] = tmp16i;
  wrbuff16[4] = tmp16;
  wrbuff16[6] = tmp16i;

  ptw = loadsize / geom.data_bytes_per_page;
  if ((ptw * geom.data_bytes_per_page) < loadsize)
  {
	ptw++;
  }
  ptw++; /* Include non-used sector */

  tmp16 = (UNS_16) ptw;
  tmp16i = 0x00FF - tmp16;
  wrbuff16[8] = tmp16;
  wrbuff16[10] = tmp16i;
  wrbuff16[12] = tmp16;
  wrbuff16[14] = tmp16i;
  wrbuff16[16] = tmp16;
  wrbuff16[18] = tmp16i;
  wrbuff16[20] = tmp16;
  wrbuff16[22] = tmp16i;
  wrbuff16[24] = 0x00AA; /* Good block marker for page #0 ICR only */

  /* Get location where to write ICR */
  progblk = 0;
  sector = progblk * geom.pages_per_block;

  /* Write ICR data to first usable block/page #0 */
  slcnand_write_sector(nand, sector, wrbuff8, 0);

  /* Verify page #0 */
  slcnand_read_sector(nand, sector, rdbuff8, 0);
  idx = isblksame(rdbuff8, wrbuff8);
  if (idx >= 0)
  {
	while (1);
  }

  progblk = 0;
  sector = progblk * geom.pages_per_block + 1;


  nand_write_sectors(sector, p8, loadsize);
  idx = verify_data(sector, p8, rdbuff8, loadsize);
  if(idx >= 0){
//...failed    
    while(1)
      asm("nop");
  }


//...Successful
  slcnand_close(nand);
  
#endif  

}



