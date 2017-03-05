/***********************************************************************
 * $Id: sd_main.c 30870 2009-07-05 10:16:16Z anderslu $
 *
 * Project: UART driver example
 *
 * Description:
 *     A simple UART driver example.
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
#include <stdio.h>
#include <string.h>
#include <NXP\iolpc3131.h>

#include "lpc_types.h"
//#include "lpc_irq_fiq.h"
//#include "lpc_arm922t_cp15_driver.h"
//#include "ea3131_board.h"
//#include "lpc313x_timer_driver.h"
#include "lpc313x_mci_driver.h"
//#include "lpc313x_ioconf_driver.h"
//#include "lpc313x_cgu_driver.h"
#include "arm_comm.h"
#include "uart.h"
#include "fat.h"


#ifdef offsetof
#undef offsetof
#endif

#define offsetof(s,m)   (int)&(((s *)0)->m)
#define COMPILE_TIME_ASSERT(expr)   char constraint[expr]

//COMPILE_TIME_ASSERT(offsetof(MCI_REGS_T, hcon) == 0x70);
//COMPILE_TIME_ASSERT(offsetof(MCI_REGS_T, data) == 0x100);


//char str_buff[512];
INT_32 g_mcidev;
//INT_32 g_uartdev;

extern void Timer1_wait_ticks(UNS_32 ticks);

static int Card_Presence(void)
{
  IOCONF_EBI_MCI_M0_CLR = (1<<31);
  IOCONF_EBI_MCI_M1_CLR = (1<<31);
  for(volatile int i = 0; i<5000; i++);

  if(IOCONF_EBI_MCI_PIN & (1<<31)) return 1;
  return 0;
}

/***********************************************************************
 *
 * Function: blkdev_init
 *
 * Purpose: SDMMC init
 *
 * Processing:
 *     Initialize the SDMMC card
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 blkdev_init(void)
{
  MCI_CARD_INFO_T* psdcard ;
  int i = 0;

  /* enable power to SD card slot by setting I2STX_DATA0 low. */
  //IOCONF_EBI_I2STX_0_M1_SET = (1<<5);
  //IOCONF_EBI_I2STX_0_M0_CLR = (1<<5);

  /* wait atleast 10msec for card to stablize */
        g_mcidev = mci_open((void*)SD_MMC_BASE, NULL);
        psdcard = (MCI_CARD_INFO_T*)g_mcidev;
       /* print card info */
        PRINTF( "%s card acquired\r\n", (psdcard->card_type & CARD_TYPE_SD) ? "SD" : "MMC");
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
        }

  return 1;
}

/***********************************************************************
 *
 * Function: blkdev_read
 *
 * Purpose: SDMMC sector read
 *
 * Processing:
 *     SDMMC sector read
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns:
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 blkdev_read(void *buff, UNS_32 sector, UNS_32 sec_cnt)
{
	if (mci_read_blocks(g_mcidev,
                       buff,
                       sector,
                       sector + sec_cnt) < 0)
	{
		return FALSE;
	}
	
	return TRUE;
}

/***********************************************************************
 *
 * Function: blkdev_deinit
 *
 * Purpose: SDMMC deinit
 *
 * Processing:
 *     De-initialize the SDMMC card
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 blkdev_deinit(void)
{
	if (g_mcidev != 0)
	{
		mci_close(g_mcidev);
		g_mcidev = 0;
	}

	return TRUE;
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
 * Returns: Always returns 0
 *
 * Notes: None
 *
 **********************************************************************/
int sd_entry(void)
{
	char ch = 0;
  int card_presence = 0 ;
  Int32U lp;
  Int8U buff[100];

  PRINTF("Press Esc for exit\n\r");

  PRINTF("Please insert card\r\n");

  while('\x1B' != ch)
  {
    Timer1_wait_ticks(5000);

    if(Card_Presence())
    {
      if(!card_presence)
      {
        for(volatile int k = 0 ; 10000 > k;k++);
        card_presence = 1;
         if (fat_init(blkdev_init, blkdev_read, blkdev_deinit) == FALSE)
          {
            PRINTF("fat_init failed\r\n");
          }
          else
          {
            /* Show items in BLKDEV root directory */
            lp = fat_get_dir((UNS_8*)buff, TRUE);
            while (lp == FALSE)
            {
              PRINTF(" ");
              PRINTF(buff);
              PRINTF("\r\n");

              lp = fat_get_dir((UNS_8*)buff, FALSE);
            }
            /* read firmware.bin file if present on drive*/

            fat_deinit();
          }
          mci_close(g_mcidev);
      }
    }
    else
    {
      if(card_presence)
      {
        card_presence = 0;
        PRINTF("Please insert card\r\n");
      }
    }

    UartRead((unsigned char *)&ch,1);
  }

  blkdev_deinit();

  return 0;
}

