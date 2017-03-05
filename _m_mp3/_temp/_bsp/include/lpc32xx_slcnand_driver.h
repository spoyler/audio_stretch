/***********************************************************************
 * $Id:: lpc32xx_slcnand_driver.h 8090 2011-09-14 13:57:49Z ing03005   $
 *
 * Project: LPC32xx SLC NAND controller driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx SLC NAND
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

#ifndef LPC32XX_SLCNAND_DRIVER_H
#define LPC32XX_SLCNAND_DRIVER_H

#include "lpc32xx_dma_driver.h"
#include "lpc32xx_slcnand.h"
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
 * SLC NAND controller device configuration commands (IOCTL commands
 * and arguments)
 **********************************************************************/

/* Command and address FIFO population structure */
typedef struct
{
  int num_items;      /* Number of command or address bytes */
  UNS_8 items[16];    /* Commands or addresses, 16 max */
} SLC_CMDADR_T;

/* SLC NAND timing control structure - used to set the number of clocks
   for each NAND signal timing component */
typedef struct
{
  UNS_32 w_rdy;   /* The time before the signal RDY is tested in terms
                     of 2 * clock cycles. After these 2*W_RDY[2:0]
                     clocks, RDY is sampled by the interface.
                     If RDY = 0, the bus sequencer stops. RDY is
                     sampled on each clock until it equals 1, then
                     the bus sequencer continues. */
  UNS_32 w_width; /* Write pulse width in clock cycles.
                     Programmable from 1 to 16 clocks. */
  UNS_32 w_hold;  /* Write hold time of ALE, CLE, CEn, and Data in
                     clock cycles. Programmable from 1 to 16
                     clocks. */
  UNS_32 w_setup; /* Write setup time of ALE, CLE, CEn, and Data in
                     clock cycles. Programmable from 1 to 16
                     clocks. */
  UNS_32 r_rdy;   /* Time before the signal RDY is tested in terms
                     of 2 * clock cycles. After these 2*R_RDY[2:0]
                     cycles, RDY is sampled by the interface.
                     If RDY = 0, the bus sequencer stops. RDY is
                     sampled on each clock until it equals 1, then
                     the bus sequencer continues. */
  UNS_32 r_width; /* Read pulse in clock cycles. Programmable from
                     1 to 16 clocks. */
  UNS_32 r_hold;  /* Read hold time of ALE, CLE, and CEn in clock
                     cycles. Programmable from 1 to 16 clocks. */
  UNS_32 r_setup; /* Read setup time of ALE, CLE, and CEn in clock
                     cycles. Programmable from 1 to 16 clocks. */
} SLC_TAC_T;

/* SLC NAND controller arguments for SLC_GET_STATUS command (IOCTL
   arguments), see argument TBD */
typedef enum
{
  DMA_FIFO_ST,     /* When used as arg, will return a '0' if SLC DMA
                      FIFO is empty, otherwise nonzero */
  SLC_FIFO_ST,     /* When used as arg, will return a '0' if SLC data
                      FIFO is empty, otherwise nonzero */
  SLC_READY_ST,    /* When used as arg, will return a '0' if SLC NAND
                      ready signal is low (busy), otherwise nonzero */
  SLC_INT_ST       /* When used as arg, will return the interrupt
                      states */
} SLC_IOCTL_STS_T;

/* SLC NAND flash block/page structure */
typedef struct
{
  DMAC_LL_T *dma;     /* DMA link list pointer
                         if 0 then dma disabled
                         else dma enabled */
  UNS_32 *ecc;        /* ECC buffer pointer
                         if 0 then ecc disabled
                         else ecc enabled */
  UNS_32 block_num;   /* Block number */
  UNS_32 page_num;    /* Page number */
  UNS_8 *buffer;      /* buffer pointer */
} SLC_BLOCKPAGE_T;

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
 * SLC NAND driver API functions
 **********************************************************************/

/* Open the SLC NAND controller */
INT_32 slcnand_open(void *ipbase, INT_32 arg) FUNC_SECTION(".slcopen");

/* Close the SLC NAND controller */
STATUS slcnand_close(INT_32 devid) FUNC_SECTION(".slcclose");

/* Board initialization routine provided by every bsp */
INT_32 slcnand_board_init(void) FUNC_SECTION(".slcbrdinit");

/* NAND sector read/write routine */
STATUS slcnand_read_sector(INT_32 devid, INT_32 sector,
    UNS_8 *data, UNS_8 *spare) FUNC_SECTION(".slcwrsec");
STATUS slcnand_write_sector(INT_32 devid, INT_32 sector,
    UNS_8 *data, UNS_8 *spare) FUNC_SECTION(".slcrdsec");
STATUS slcnand_erase_block(INT_32 devid, UNS_32 block_num) FUNC_SECTION(".slceraseblk");

/* Bad block management functions */
INT_32 slcnand_is_block_bad(INT_32 devid, INT_32 block) FUNC_SECTION(".slcisbad");
INT_32 slcnand_mark_block_bad(INT_32 devid, INT_32 block) FUNC_SECTION(".slcmarkbad");

/* Misc. functions */
INT_32 slcnand_get_geom(INT_32 devid, NAND_GEOM_T *geom) FUNC_SECTION(".slcgetgeom");
INT_32 slcnand_get_block(INT_32 devid, INT_32 sector) FUNC_SECTION(".slcgetblk");
INT_32 slcnand_get_page(INT_32 devid, INT_32 sector) FUNC_SECTION(".slcgetpg");

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_SLCNAND_DRIVER_H */
