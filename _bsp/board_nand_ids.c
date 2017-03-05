/***********************************************************************
 * $Id:: board_nand_ids.c 7833 2011-08-05 04:49:37Z ing03005 $
 *
 * Project: NAND Devices list for EA3250 board
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
#include "lpc32xx_slcnand_driver.h"
#include "lpc32xx_mlcnand_driver.h"
#include "lpc_nandflash_params.h"
#include "phy3250_board.h"

/* Timing parameter */
#define PHY3250_SLC_TIMING \
		SLCTAC_WDR(SLC_NAND_W_RDY) | \
		SLCTAC_WWIDTH(SLC_NAND_W_WIDTH) | \
		SLCTAC_WHOLD(SLC_NAND_W_HOLD) | \
		SLCTAC_WSETUP(SLC_NAND_W_SETUP) | \
		SLCTAC_RDR(SLC_NAND_R_RDY) | \
		SLCTAC_RWIDTH(SLC_NAND_R_WIDTH) | \
		SLCTAC_RHOLD(SLC_NAND_R_HOLD) | \
		SLCTAC_RSETUP(SLC_NAND_R_SETUP)

#define PHY3250_MLC_TIMING \
		(MLC_LOAD_TCEA(MLC_TCEA_TIME) | \
		MLC_LOAD_TWBTRB(MLC_TWBTRB_TIME) | \
        MLC_LOAD_TRHZ(MLC_TRHZ_TIME) | \
		MLC_LOAD_TREH(MLC_TREH_TIME) | \
        MLC_LOAD_TRP(MLC_TRP_TIME) | \
		MLC_LOAD_TWH(MLC_TWH_TIME) | \
		MLC_LOAD_TWP(MLC_TWP_TIME))

#define SLCNAND_STMICRO_TIMING PHY3250_SLC_TIMING
#define MLCNAND_STMICRO_TIMING PHY3250_MLC_TIMING

static struct nand_ids phy3250_nand_ids[] =
{
  /* ST MICRO Flash */
  {
    LPCNAND_VENDOR_STMICRO,
    0x73, /* Device ID */
    512, /* Page size */
    32, /* Pages per block */
    1024, /* Total blocks */
	{SLCNAND_STMICRO_TIMING, MLCNAND_STMICRO_TIMING},/* Timing */
  },

  /* ST MICRO Flash */
  {
    LPCNAND_VENDOR_STMICRO,
    0x35, /* Device ID */
    512, /* Page size */
    32, /* Pages per block */
    2048, /* Total blocks */
	{SLCNAND_STMICRO_TIMING, MLCNAND_STMICRO_TIMING},/* Timing */
  },

  /* ST MICRO Flash */
  {
    LPCNAND_VENDOR_STMICRO,
    0x75, /* Device ID */
    512, /* Page size */
    32, /* Pages per block */
    2048, /* Total blocks */
	{SLCNAND_STMICRO_TIMING, MLCNAND_STMICRO_TIMING},/* Timing */
  },

  /* ST MICRO Flash */
  {
    LPCNAND_VENDOR_STMICRO,
    0x36, /* Device ID */
    512, /* Page size */
    32, /* Pages per block */
    4096, /* Total blocks */
	{SLCNAND_STMICRO_TIMING, MLCNAND_STMICRO_TIMING},/* Timing */
  },

  /* ST MICRO Flash */
  {
    LPCNAND_VENDOR_STMICRO,
    0x76, /* Device ID */
    512, /* Page size */
    32, /* Pages per block */
    4096, /* Total blocks */
	{SLCNAND_STMICRO_TIMING, MLCNAND_STMICRO_TIMING},/* Timing */
  },

  /* ST MICRO Flash */
  {
    LPCNAND_VENDOR_STMICRO,
    0x39, /* Device ID */
    512, /* Page size */
    32, /* Pages per block */
    8192, /* Total blocks */
	{SLCNAND_STMICRO_TIMING, MLCNAND_STMICRO_TIMING},/* Timing */
  },

  /* ST MICRO Flash */
  {
    LPCNAND_VENDOR_STMICRO,
    0x79, /* Device ID */
    512, /* Page size */
    32, /* Pages per block */
    8192, /* Total blocks */
	{SLCNAND_STMICRO_TIMING, MLCNAND_STMICRO_TIMING},/* Timing */
  },
  
  {
    MICRON_2Gb,
    0xDA, /* Device ID */
    2048, /* Page size */
    64, /* Pages per block */
    2048, /* Total blocks */
    {SLCNAND_STMICRO_TIMING, MLCNAND_STMICRO_TIMING},/* Timing */
  },  
};

static struct nand_devinfo_list phy3250_nand_list =
{
  sizeof(phy3250_nand_ids)/sizeof(phy3250_nand_ids[0]),
  phy3250_nand_ids,
  NULL
};

/***********************************************************************
 *
 * Function: slcnand_board_init
 *
 * Purpose: Intializes the SLC NAND contoller and the listed NAND Device
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Device ID on success
 *          0 on Failure
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slcnand_board_init(void)
{
  return slcnand_open(SLCNAND, (INT_32)&phy3250_nand_list);
}

/***********************************************************************
 *
 * Function: mlcnand_board_init
 *
 * Purpose: Intializes the MLC NAND contoller and the listed NAND Device
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Device ID on success
 *          0 on Failure
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mlcnand_board_init(void)
{
  return mlcnand_open(MLCNAND, (INT_32)&phy3250_nand_list);
}
