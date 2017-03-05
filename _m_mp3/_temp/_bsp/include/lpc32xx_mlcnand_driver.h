/***********************************************************************
 * $Id:: lpc32xx_mlcnand_driver.h 8090 2011-09-14 13:57:49Z ing03005   $
 *
 * Project: LPC32xx MLC NAND controller driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx MLC NAND
 *     controller.
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
 *********************************************************************/

#ifndef LPC32XX_MLCNAND_DRIVER_H
#define LPC32XX_MLCNAND_DRIVER_H

#include "lpc32xx_mlcnand.h"
#include "lpc32xx_dma_driver.h"
#include "lpc_nandflash_params.h"

/* Optimization to remove unwanted functions during
 * linking using Keil tool-chain.  */
#ifndef FUNC_SECTION
# ifdef __ARMCC_VERSION
# define FUNC_SECTION(x) __attribute__ ((section (x)))
#else
# define FUNC_SECTION(x)
# endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * MLC NAND controller device configuration commands (IOCTL commands
 * and arguments)
 **********************************************************************/

/* Command and address FIFO population structure */
typedef struct
{
  int num_items;      /* Number of command or address bytes */
  UNS_8 items[16];    /* Commands or addresses, 16 max */
} MLC_CMDADR_T;

/* MLC NAND timing control structure - used to set the number of clocks
   for each NAND signal timing component */
typedef struct
{
  UNS_32 tcea_delay; /* nCE low to dout valid (tCEA). */
  UNS_32 busy_delay; /* Read/Write high to busy (tWB/tRB). */
  UNS_32 nand_ta;    /* Read high to high impedance (tRHZ). */
  UNS_32 r_high;     /* Read high hold time (tREH) */
  UNS_32 r_low;      /* Read pulse width (tRP) */
  UNS_32 wr_high;    /* Write high hold time (tWH) */
  UNS_32 wr_low;     /* Write pulse width (tWP) */
} MLC_TIMING_T;

/* MLC NAND controller arguments for MLC_GET_STATUS command (IOCTL
   arguments), see argument TBD */
typedef enum
{
  MLC_ST,          /* When used as arg, will return the states */
  MLC_INT_ST       /* When used as arg, will return the interrupt
                      states */
} MLC_IOCTL_STS_T;

/* MLC NAND flash block/page structure */
typedef struct
{
  DMAC_LL_T *dma;     /* DMA link list pointer
                         if 0 then dma disabled
                         else dma enabled */
  UNS_32 block_num;   /* Block number */
  UNS_32 page_num;    /* Page number */
  UNS_8 *buffer;      /* buffer pointer */
} MLC_BLOCKPAGE_T;

#ifndef NAND_ID_STRUCT
#define NAND_ID_STRUCT
/* NAND Device specific defines */
#define NAND_LB_BADBLOCK_OFFS  0
#define NAND_SB_BADBLOCK_OFFS  5
#define NAND_GOOD_BLOCK_MARKER 0xFF
struct nand_timing
{
	UNS_32 slc;
	UNS_32 mlc;
};

/* NAND ID specific structures */
struct nand_ids
{
  UNS_8 ven_id;              /* Vendor ID of NAND device */
  UNS_8 dev_id;              /* Device ID of NAND device */
  INT_32 page_size;          /* Page size */
  INT_32 pages_per_block;    /* Pages per erase block */
  INT_32 total_blocks;       /* Total number of blocks */
  struct nand_timing timing; /* Timing parameters for NAND */
};

/* NAND Device registration structure */
struct nand_devinfo_list
{
  INT_32 count;                  /* Number of IDs present */
  struct nand_ids * ids;         /* Array of NAND IDs */
  UNS_32(*virt_to_phy)(void *);  /* Virtual to physical function */
};
#endif

/***********************************************************************
 * MLC NAND driver API functions
 **********************************************************************/

/* Open the MLC NAND controller */
INT_32 mlcnand_open(void *ipbase, INT_32 arg) FUNC_SECTION(".mlcopen");

/* Close the MLC NAND controller */
STATUS mlcnand_close(INT_32 devid) FUNC_SECTION(".mlcclose");

/* MLC NAND controller configuration block */
STATUS mlcnand_ioctl(INT_32 devid,
                     INT_32 cmd,
                     INT_32 arg) FUNC_SECTION(".mlcioctl");

/* MLC NAND controller read function (non-DMA only) */
INT_32 mlcnand_read(INT_32 devid,
                    void *buffer,
                    INT_32 max_bytes) FUNC_SECTION(".mlcread");

/* MLC NAND controller write function (non-DMA only) */
INT_32 mlcnand_write(INT_32 devid,
                     void *buffer,
                     INT_32 n_bytes) FUNC_SECTION(".mlcwrite");

/* Open the SLC NAND controller */
INT_32 mlcnand_open(void *ipbase, INT_32 arg) FUNC_SECTION(".mlcopen");

/* Close the SLC NAND controller */
STATUS mlcnand_close(INT_32 devid) FUNC_SECTION(".mlcclose");

/* Board initialization routine provided by every bsp */
INT_32 mlcnand_board_init(void) FUNC_SECTION(".mlcinit");

/* NAND sector read/write routine */
STATUS mlcnand_read_sector(INT_32 devid, INT_32 sector,
    UNS_8 *data, UNS_8 *spare) FUNC_SECTION(".mlcrdsec");
STATUS mlcnand_write_sector(INT_32 devid, INT_32 sector,
    UNS_8 *data, UNS_8 *spare) FUNC_SECTION(".mlcwrsec");
STATUS mlcnand_erase_block(INT_32 devid, UNS_32 block_num) FUNC_SECTION(".mlcerase");

/* Bad block management functions */
INT_32 mlcnand_is_block_bad(INT_32 devid, INT_32 block) FUNC_SECTION(".mlcisbad");
INT_32 mlcnand_mark_block_bad(INT_32 devid, INT_32 block) FUNC_SECTION(".mlcmarkbad");

/* Misc. functions */
INT_32 mlcnand_get_geom(INT_32 devid, NAND_GEOM_T *geom) FUNC_SECTION(".mlcgetgeom");
INT_32 mlcnand_get_block(INT_32 devid, INT_32 sector) FUNC_SECTION(".mlcgetblock");
INT_32 mlcnand_get_page(INT_32 devid, INT_32 sector) FUNC_SECTION(".mlcgetpg");

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_MLCNAND_DRIVER_H */
