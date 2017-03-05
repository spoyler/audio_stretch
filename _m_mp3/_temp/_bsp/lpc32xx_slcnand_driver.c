/***********************************************************************
 * $Id:: lpc32xx_slcnand_driver.c 8093 2011-09-14 16:06:29Z ing03005   $
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

#include <string.h>
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_dma_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc_nandflash_params.h"
#include "lpc32xx_slcnand_driver.h"
#include "lpc_arm922t_cp15_driver.h"

/***********************************************************************
 * SLC NAND controller driver package data
***********************************************************************/

typedef struct slcnand_dev
{
  UNS_8  data_buff[2048];
  UNS_8  spare_buff[64];
  UNS_32 ecc[8];
  DMAC_LL_T dmalist[17];
  INT_32 dmach;
  SLCNAND_REGS_T *regs;
  INT_32 initialized;
  INT_32 page_size;
  INT_32 block_size;
  INT_32 chip_size;
  INT_32 spare_size;
  INT_32 pages_per_block;
  INT_32 block_count;
  INT_32 large_page;
  UNS_32(*v2p)(void *);
}SLCNAND_DEV_T;

/* For now we support only one device */
static /*DMA_BUFFER*/ SLCNAND_DEV_T slcnand_device[1];

static volatile INT_32 dma_xfer_status = 0;

/* Layout of ECC in NAND flash OOB */
static int sp_ooblayout[] =
{
  10, 11, 12, 13, 14, 15
};

static int lp_ooblayout[] =
{
  40, 41, 42, 43, 44, 45,
  46, 47, 48, 49, 50, 51,
  52, 53, 54, 55, 56, 57,
  58, 59, 60, 61, 62, 63
};

/***********************************************************************
 * SLC NAND controller driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: slcnand_cmd
 *
 * Purpose: Writes an NAND command to the given NAND flash device
 *
 * Processing:
 *     See function.
 *
 * Parameters: dev -> NAND Device data structure
 *             cmd -> Command to be sent to NAND device
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
static void slcnand_cmd(const SLCNAND_DEV_T *dev, UNS_32 cmd)
{
  dev->regs->slc_cmd = cmd;
}

/***********************************************************************
 *
 * Function: slcnand_status
 *
 * Purpose: Reads the status of NAND operation like erase / program
 *
 * Processing:
 *     See function.
 *
 * Parameters: dev -> NAND Device data structure
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR -> Operation OK
 *             _ERROR -> Error during operation
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS slcnand_status(const SLCNAND_DEV_T *dev)
{
  STATUS status = _NO_ERROR;

  /* Status read (1) command */
  slcnand_cmd(dev, NAND_CMD_STATUS);

  /* Operation status */
  if (dev->regs->slc_data & NAND_FLASH_FAILED)
  {
    status = _ERROR;
  }
  return status;
}

/***********************************************************************
 *
 * Function: slcnand_wait_ready
 *
 * Purpose: Busy wait on NAND ready
 *
 * Processing:
 *     See function.
 *
 * Parameters: dev -> NAND Device data structure
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
static void slcnand_wait_ready(const SLCNAND_DEV_T *dev)
{
  /* Wait for SLC NAND ready */
  while (!(dev->regs->slc_stat & SLCSTAT_NAND_READY));
}


/***********************************************************************
 *
 * Function: slcnand_wait_ready
 *
 * Purpose: Busy wait on NAND ready
 *
 * Processing:
 *     See function.
 *
 * Parameters: dev -> NAND Device data structure
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
static void slcnand_addr(const SLCNAND_DEV_T *dev, UNS_32 addr)
{
  dev->regs->slc_addr = addr;
}

/***********************************************************************
 *
 * Function: slcnand_v2p
 *
 * Purpose: Covert an virtual address to physical address
 *
 * Processing:
 *     See function.
 *
 * Parameters: vaddr -> virtual address to be converted
 *
 * Outputs: None
 *
 * Returns: Physical address corresponding to vaddr
 *
 * Notes: None
 *
 **********************************************************************/
static UNS_32 slcnand_v2p(void *vaddr)
{
  return (UNS_32) vaddr;
}

/***********************************************************************
 *
 * Function: slcnand_dma_interrupt
 *
 * Purpose: DMA controller interrupt handler
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
static void slcnand_dma_interrupt(int error)
{
  if (!error)
    dma_xfer_status = 1;
  else
    dma_xfer_status = -1;
}

/***********************************************************************
 *
 * Function: slcnand_wait_dma
 *
 * Purpose: Waits for DMA completion
 *
 * Processing:
 *     See function.
 *
 * Parameters: dev -> slc nand device pointer
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS slcnand_wait_dma(const SLCNAND_DEV_T * dev)
{
  STATUS stat = _NO_ERROR;
  while (!dma_xfer_status)
    ;
  if (dma_xfer_status < 0)
    stat = _ERROR;
  dma_xfer_status = 0;

  return stat;
}

/***********************************************************************
 *
 * Function: slcnand_dev
 *
 * Purpose: Converts device handle to pointer to device datastructure
 *
 * Processing:
 *     See function.
 *
 * Parameters: devid -> NAND Device id
 *
 * Outputs: None
 *
 * Returns: NULL on Error
 *          Pointer to SLCNAND_DEV_T on success
 *
 * Notes: None
 *
 **********************************************************************/
static SLCNAND_DEV_T *slcnand_dev(INT_32 devid)
{
  SLCNAND_DEV_T *dev;
  if (!devid)
    return 0;

  dev = (SLCNAND_DEV_T *) devid;
  if (!dev->initialized)
    return 0;
  return (SLCNAND_DEV_T *)devid;
}

/***********************************************************************
 *
 * Function: slcnand_status
 *
 * Purpose: Copies ECC stored in Spare area to integer array
 *
 * Processing:
 *     See function.
 *
 * Parameters: spare -> pointer to spare area
 *             ecc -> pointer to integer array
 *             eccCount -> Number of ecc words
 *
 * Outputs: None
 *
 * Returns: 0 -> Error
 *          1 -> No Error
 *
 * Notes: None
 *
 **********************************************************************/
static int slcnand_ecc_to_buffer(
  unsigned char *spare,
  const unsigned long *ecc,
  int eccCount)
{
  int * offset, i;
  if (eccCount == 2)
  {
    offset = sp_ooblayout;
  }
  else if (eccCount == 8)
  {
    offset = lp_ooblayout;
  }
  else
  {
    return 0;
  }

  for (i = 0; i < (eccCount * 3); i += 3)
  {
    unsigned long ce = ecc[i/3];
    ce = ~(ce << 2) & 0xFFFFFF;
    spare[offset[i+2]] = (unsigned char)(ce & 0xFF);
    ce >>= 8;
    spare[offset[i+1]] = (unsigned char)(ce & 0xFF);
    ce >>= 8;
    spare[offset[i]]   = (unsigned char)(ce & 0xFF);
  }
  return 1;
}

/***********************************************************************
 *
 * Function: slcnand_ecc_from_buffer
 *
 * Purpose: Copies ECC stored in Spare area to integer array
 *
 * Processing:
 *     See function.
 *
 * Parameters: spare -> pointer to spare area
 *             ecc -> pointer to integer array
 *             eccCount -> Number of ecc words
 *
 * Outputs: None
 *
 * Returns: 0 -> Error
 *          1 -> No Error
 *
 * Notes: None
 *
 **********************************************************************/
static int slcnand_ecc_from_buffer(const unsigned char *spare,
                                   unsigned long *ecc, int eccCount)
{
  int * offset, i;
  if (eccCount == 2)
  {
    offset = sp_ooblayout;
  }
  else if (eccCount == 8)
  {
    offset = lp_ooblayout;
  }
  else
  {
    return 0;
  }

  for (i = 0; i < (eccCount * 3); i += 3)
  {
    unsigned long ce = 0;
    ce |= spare[offset[i]];
    ce <<= 8;
    ce |= spare[offset[i+1]];
    ce <<= 8;
    ce |= spare[offset[i+2]];
    ecc[i/3] = (~ce >> 2) & 0x3FFFFF;
  }
  return 1;
}

/***********************************************************************
 *
 * Function: bic_cntxx
 *
 * Purpose: Counts the number of 1's in given (half)word
 *
 * Processing:
 *     See function.
 *
 * Parameters: ch -> (half)word input
 *
 * Outputs: None
 *
 * Returns: Number of 1's in ch
 *
 * Notes: None
 *
 **********************************************************************/
static int bit_cnt16(unsigned short ch)
{
  ch = (ch & 0x5555) + ((ch & ~0x5555) >> 1);
  ch = (ch & 0x3333) + ((ch & ~0x3333) >> 2);
  ch = (ch & 0x0F0F) + ((ch & ~0x0F0F) >> 4);
  return (ch + (ch >> 8)) & 0xFF;
}

static int bit_cnt32(unsigned int val)
{
  return bit_cnt16((unsigned short)(val & 0xFFFF)) +
         bit_cnt16((unsigned short)(val >> 16));
}

/***********************************************************************
 *
 * Function: slcnand_ecc_correct256
 *
 * Purpose: Detects and corrects ECC errors in a 256 byte block
 *
 * Processing:
 *     See function.
 *
 * Parameters: ecc_gen -> Generated ECC
 *             ecc_stored -> Stored ECC
 *             buf -> pointer to data buffer
 *
 * Outputs: None
 *
 * Returns: 0 -> NO Error
 *          -1 -> uncorrectable error
 *          -2 -> Error in Spare area
 *          1 -> Error detected and corrected
 *
 * Notes: None
 *
 **********************************************************************/
static int slcnand_ecc_correct256(
unsigned int ecc_gen,
unsigned int ecc_stored,
unsigned char * buf)
{
  unsigned int tmp, err;
  err = ecc_stored ^ ecc_gen;

  /* Return if ECC is OK */
  if (!err)
    return 0;

  /* ECC Failure in i-th block */
  tmp = bit_cnt32(err);
  if (tmp == 11)
  {
    unsigned int byte = err >> 6;
    unsigned int bit = 0;
    bit = ((err & _BIT(1)) >> 1) | ((err & _BIT(3)) >> 2) |
          ((err & _BIT(5)) >> 3);

    /* Calculate Byte offset */
    byte = ((byte & _BIT(1)) >> 1) | ((byte & _BIT(3)) >> 2) |
           ((byte & _BIT(5)) >> 3) | ((byte & _BIT(7)) >> 4) |
           ((byte & _BIT(9)) >> 5) | ((byte & _BIT(11)) >> 6) |
           ((byte & _BIT(13)) >> 7) | ((byte & _BIT(15)) >> 8);

    /* Do the correction */
    buf[byte] ^= _BIT(bit);
    return 1;
  }
  else if (tmp == 1)
  {
    /* Error in OOB area */
    return -2;
  }
  else
  {
    return -1;
  }
}

/***********************************************************************
 *
 * Function: slcnand_read_id
 *
 * Purpose: Read SLC NAND flash id
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as slc was not
 *     previously opened. Otherwise, read SLC NAND flash id.
 *
 * Parameters:
 *     dev:     (IN) Pointer to SLC NAND controller config structure
 *     man_id:  (OUT) NAND Manufacturer ID
 *     dev_id:  (OUT) NAND Device ID
 *
 * Outputs: None
 *
 * Returns: The status of read id operation
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS slcnand_read_id(
    const SLCNAND_DEV_T *dev, /* IN: NAND Device structure */
    UNS_8 *man_id,            /* OUT: NAND manufacture ID */
    UNS_8 *dev_id)            /* OUT: NAND Device ID */
{
  volatile int tmp;

  if (!dev->regs)
    return _ERROR;

  slcnand_cmd(dev, NAND_CMD_READID);
  slcnand_addr(dev, 0);
  *man_id = dev->regs->slc_data;
  *dev_id = dev->regs->slc_data;
  tmp = dev->regs->slc_data;
  tmp = dev->regs->slc_data;

  return _NO_ERROR;
}

/***********************************************************************
 *
 * Function: slcnand_identify_device
 *
 * Purpose: Identifies a NAND device based on its VENDOR and DEVICE IDs
 *
 * Processing:
 *     See function.
 *
 * Parameters: dev -> NAND Device data structure
 *
 * Outputs: None
 *
 * Returns: pointer to device geometry structure
 *
 * Notes: None
 *
 **********************************************************************/
static struct nand_ids * slcnand_identify_device(
    const SLCNAND_DEV_T *dev,
    const struct nand_devinfo_list *dlst)
{
  int i;
  UNS_8 mfid, devid;
  if (slcnand_read_id(dev, &mfid, &devid) == _ERROR)
    return 0;
  for (i = 0; i < dlst->count; i++)
  {
    if (mfid == dlst->ids[i].ven_id && devid == dlst->ids[i].dev_id)
      return &dlst->ids[i];
  }
  return 0;
}

/***********************************************************************
 *
 * Function: slcnand_write_addr
 *
 * Purpose: Writes the given column and sector address of NAND
 *
 * Processing:
 *     See function.
 *
 * Parameters: dev -> NAND Device data structure
 *             column -> Offset with a page (-1 -> to skip)
 *             sector -> Sector number of the page
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: For operations that involves only page address column
 *        should be -1 (Example erase oparation).
 *
 **********************************************************************/
static void slcnand_write_addr(
    const SLCNAND_DEV_T *dev,
    INT_32 column,
    UNS_32 sector)
{
  /* Write column address if given */
  if (column != -1)
  {
    slcnand_addr(dev, column & 0xFF);
    if (dev->large_page)
      slcnand_addr(dev, ((UNS_32)column >> 8) & 0xFF);
  }

  /* Write the page address */
  slcnand_addr(dev, sector & 0xFF);
  slcnand_addr(dev, (sector >> 8) & 0xFF);

  /* For small page devices of size > 32 MiB
   * and large page devices of size > 128 MiB
   * We need one more address cycle */
  if ((!dev->large_page && dev->chip_size > (32 << 20)) ||
      (dev->large_page && dev->chip_size > (128 << 20)))
    slcnand_addr(dev, (sector >> 16) & 0xFF);
}

/***********************************************************************
 *
 * Function: slcnand_start_dma
 *
 * Purpose: Starts a NAND DMA transfer
 *
 * Processing:
 *     See function.
 *
 * Parameters: dev -> NAND Device data structure
 *             desc_index -> index to the descriptor
 *             config -> DMA config options
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
static void slcnand_start_dma(
    SLCNAND_DEV_T *dev,
    INT_32 desc_index,
    UNS_32 config)
{
  DMAC_REGS_T *pdmaregs;
  DMAC_LL_T *dmalst = &dev->dmalist[desc_index];
  INT_32 dmach = dev->dmach;

  /* setup DMA channel */
  pdmaregs = dma_get_base();
  pdmaregs->int_tc_clear = _BIT(0);
  pdmaregs->int_err_clear = _BIT(0);
  pdmaregs->int_tc_clear = _BIT(dmach);
  pdmaregs->int_err_clear = _BIT(dmach);
  pdmaregs->dma_chan[dmach].src_addr = dmalst->dma_src;
  pdmaregs->dma_chan[dmach].dest_addr = dmalst->dma_dest;
  pdmaregs->dma_chan[dmach].lli = dmalst->next_lli;
  pdmaregs->dma_chan[dmach].control = dmalst->next_ctrl;
  pdmaregs->dma_chan[dmach].config_ch = config;
}

/***********************************************************************
 *
 * Function: slcnand_prepare_dma
 *
 * Purpose: Prepares DMA descriptors for NAND read/write transfers
 *
 * Processing:
 *     See function.
 *
 * Parameters: dev -> NAND Device data structure
 *             buff -> User buffer holding data
 *             oob -> buffer to copy oob
 *             read -> 0 to write, !0 to read
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR -> Operation OK
 *             _ERROR -> Error during operation
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS slcnand_prepare_dma(
    SLCNAND_DEV_T *dev,
    UNS_8 *buff,
    UNS_8 *oob,
    INT_32 read)
{
  STATUS status = _NO_ERROR;
  UNS_32 ctrl;
  INT_32 i;
  DMAC_LL_T * dmalst;

  /* Check if device is opened */
  if (!dev->initialized)
    return _ERROR;

  ctrl = (DMAC_CHAN_SRC_BURST_4 |
          DMAC_CHAN_DEST_BURST_4 |
          DMAC_CHAN_SRC_WIDTH_32 |
          DMAC_CHAN_DEST_WIDTH_32 |
          DMAC_CHAN_DEST_AHB1 |
          (read ? 0 : DMAC_CHAN_SRC_AHB1) |
          (read ? DMAC_CHAN_DEST_AUTOINC : DMAC_CHAN_SRC_AUTOINC));

  dmalst = dev->dmalist;

  for (i = 0; i < dev->page_size / 256; i++, dmalst++)
  {
    /* Prepare DATA descriptor */
    dmalst->dma_src = dev->v2p(read ? (void *) & dev->regs->slc_dma_data :
                               &buff[i * 256]);
    dmalst->dma_dest = dev->v2p(!read ? (void *) & dev->regs->slc_dma_data :
                                &buff[i * 256]);
    dmalst->next_ctrl = 64 | ctrl; /* RD/WR 64 words per descriptor */
    dmalst->next_lli = dev->v2p(dmalst + 1);
    dmalst ++;

    /* Prepare ECC descriptor */
    dmalst->dma_src = dev->v2p((void *) & dev->regs->slc_ecc);
    dmalst->dma_dest = dev->v2p(&dev->ecc[i]);
    dmalst->next_lli = dev->v2p(dmalst + 1);
    dmalst->next_ctrl = (0x5 |
                         DMAC_CHAN_SRC_BURST_1 |
                         DMAC_CHAN_DEST_BURST_1 |
                         DMAC_CHAN_SRC_WIDTH_32 |
                         DMAC_CHAN_DEST_WIDTH_32 |
                         DMAC_CHAN_DEST_AHB1 |
                         (read ? 0 : DMAC_CHAN_SRC_AHB1));
  }

  /* For write transfers we need to relocate & invert ECC */
  if (!read)
  {
    (dmalst - 1)->next_lli   = 0;
    (dmalst - 1)->next_ctrl |= DMAC_CHAN_INT_TC_EN;
  }

  /* Prepare Spare area descriptor */
  dmalst->dma_src = dev->v2p(read ? (void *) & dev->regs->slc_dma_data : oob);
  dmalst->dma_dest = dev->v2p(!read ? (void *) & dev->regs->slc_dma_data : oob);
  dmalst->next_lli = 0;
  dmalst->next_ctrl = (dev->spare_size / 4) | ctrl | DMAC_CHAN_INT_TC_EN;
  /* Flush out the cached descriptors */
  cp15_force_cache_coherence((void *) dev->dmalist,
    (void*)((UNS_8 *)dev->dmalist + sizeof(dev->dmalist) - 1));

  return status;
}

/***********************************************************************
 * SLC NAND controller driver public functions
 **********************************************************************/
/***********************************************************************
 *
 * Function: slcnand_erase_block
 *
 * Purpose: Erase SLC NAND flash block
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as slc was not
 *     previously opened. Otherwise, erase SLC NAND flash block.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *     block_num: Block to be erased
 *
 * Outputs: None
 *
 * Returns: The status of block erase operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS slcnand_erase_block(INT_32 devid, UNS_32 block_num)
{
  STATUS status = _ERROR;
  SLCNAND_DEV_T *dev = slcnand_dev(devid);

  if (!dev->initialized)
    return status;

  slcnand_wait_ready(dev);
  slcnand_cmd(dev, NAND_CMD_ERASE1ST);
  slcnand_write_addr(dev, -1, block_num * dev->pages_per_block);
  slcnand_cmd(dev, NAND_CMD_ERASE2ND);
  slcnand_wait_ready(dev);
  return slcnand_status(dev);
}


/***********************************************************************
 *
 * Function: slcnand_read_sector
 *
 * Purpose: Reads the sector/page from NAND device
 *
 * Processing:
 *     See function.
 *
 * Parameters: devid -> NAND device handler ID
 *             sector -> Sector number to read
 *             data -> Pointer to buffer to hold data
 *             spare -> Pointer to hold spare data
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR -> Operation OK
 *             _ERROR -> Error during operation
 *
 * Notes: @data and @spare both the pointers must be word aligned,
 *        passing a NULL to data reads only Spare area, and passing
 *        NULL to spare will read only data area.
 *
 **********************************************************************/
STATUS slcnand_read_sector(
    INT_32 devid,
    INT_32 sector,
    UNS_8 *data,
    UNS_8 *spare)
{
  int i, errcnt;
  STATUS stat;
  SLCNAND_DEV_T *dev = slcnand_dev(devid);
  UNS_32 lecc[8];
  UNS_32 config = DMAC_CHAN_ITC |
                  DMAC_CHAN_IE |
                  DMAC_CHAN_FLOW_D_P2M |
                  DMAC_DEST_PERIP(0) |
                  DMAC_SRC_PERIP(DMA_PERID_NAND1) |
                  DMAC_CHAN_ENABLE;

  if (!dev)
    return _ERROR;

  /* Enable Chip select */
  dev->regs->slc_cfg |= SLCCFG_CE_LOW;

  /* Set DMA direction to read, burst and ECC */
  dev->regs->slc_cfg |= (SLCCFG_DMA_DIR |
                         SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
                         SLCCFG_DMA_ECC);

  /* OOB only read */
  if (!data)
    data = dev->data_buff;

  /* Data only read */
  if (!spare)
    spare = dev->spare_buff;

  /* Flush the contents just to make sure the buffers are not dirty */
  cp15_force_cache_coherence((void *) data,
    (void *)((UNS_8 *) data + dev->page_size - 1));
  cp15_force_cache_coherence((void *) spare,
    (void *)((UNS_8 *) spare + dev->spare_size - 1));
  cp15_force_cache_coherence((void *) dev->ecc,
    (void *)((UNS_8 *) dev->ecc + sizeof(dev->ecc) - 1));

  /* Prepare DMA descriptors */
  slcnand_prepare_dma(dev, data, spare, 1);
  slcnand_start_dma(dev, 0, config);

  slcnand_wait_ready(dev);

  /* Issue page read1 command */
  slcnand_cmd(dev, LPCNAND_CMD_PAGE_READ1);

  /* Write address */
  slcnand_write_addr(dev, 0, sector);

  slcnand_cmd(dev, LPCNAND_CMD_PAGE_READ2);

  /* Wait for ready */
  slcnand_wait_ready(dev);

  /* Set transfer count */
  dev->regs->slc_tc = dev->page_size + dev->spare_size;
  dev->regs->slc_ctrl |= SLCCTRL_ECC_CLEAR;
  dev->regs->slc_ctrl |= SLCCTRL_DMA_START;

  /* Wait for DMA to Complete */
  stat = slcnand_wait_dma(dev);

  /* Wait for ready */
  slcnand_wait_ready(dev);

  /* Invalidate the buffers */
  cp15_force_cache_coherence((void *)data,
    (void *)((UNS_8 *)data + dev->page_size - 1));
  cp15_force_cache_coherence((void *)spare,
    (void *)((UNS_8 *)spare + dev->spare_size - 1));
  cp15_force_cache_coherence((void *) dev->ecc,
    (void *)((UNS_8 *) dev->ecc + sizeof(dev->ecc) - 1));

  dev->regs->slc_cfg &= ~(SLCCFG_DMA_DIR |
                          SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
                          SLCCFG_DMA_ECC);

  dev->regs->slc_ctrl &= ~SLCCTRL_DMA_START;
  /* Disable Chip select */
  dev->regs->slc_cfg &= ~SLCCFG_CE_LOW;

  /* Do ECC correction */
  slcnand_ecc_from_buffer(spare, (unsigned long *)lecc, dev->page_size / 256);
  for (i = 0; i < dev->page_size / 256; i++)
  {
    int err;
    err = slcnand_ecc_correct256(dev->ecc[i], lecc[i], &data[i * 256]);
    if (!err)
      continue;

    if (err < 0)
      return _ERROR;

    errcnt ++;
  }
  return stat;
}

/***********************************************************************
 *
 * Function: slcnand_write_sector
 *
 * Purpose: Write the sector/page from NAND device
 *
 * Processing:
 *     See function.
 *
 * Parameters: devid -> NAND device handler ID
 *             sector -> Sector number to read
 *             data -> Pointer to buffer that holds data
 *             spare -> Pointer that holds spare data
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR -> Operation OK
 *             _ERROR -> Error during operation
 *
 * Notes: @data and @spare both the pointers must be word aligned,
 *        passing a NULL to data write only Spare area, and passing
 *        NULL to spare will write only data area.
 *
 **********************************************************************/
STATUS slcnand_write_sector(
    INT_32 devid,
    INT_32 sector,
    UNS_8 *data,
    UNS_8 *spare)
{
  STATUS stat;
  SLCNAND_DEV_T *dev = slcnand_dev(devid);
  UNS_32 config = DMAC_CHAN_ITC |
                  DMAC_CHAN_IE |
                  DMAC_CHAN_FLOW_D_M2P |
                  DMAC_DEST_PERIP(DMA_PERID_NAND1) |
                  DMAC_SRC_PERIP(0) |
                  DMAC_CHAN_ENABLE;

  if (!dev)
    return _ERROR;

  /* OOB only write */
  if (!data)
  {
    data = dev->data_buff;
    memset(dev->data_buff, 0xFF, 2048);
  }

  /* Data only write */
  if (!spare)
  {
    spare = dev->spare_buff;
    memset(dev->spare_buff, 0xFF, 64);
  }

  /* Flush the data contents */
  cp15_force_cache_coherence((void *)data,
    (void *)((UNS_8 *)data + dev->page_size - 1));
  cp15_force_cache_coherence((void *) dev->ecc,
    (void *)((UNS_8 *) dev->ecc + sizeof(dev->ecc) - 1));

  /* Enable Chip select */
  dev->regs->slc_cfg |= SLCCFG_CE_LOW;

  /* Set DMA direction to write, burst and ECC */
  dev->regs->slc_cfg |= (SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
                         SLCCFG_DMA_ECC);
  dev->regs->slc_cfg &= ~SLCCFG_DMA_DIR;

  /* Prepare DMA descriptors */
  slcnand_prepare_dma(dev, data, spare, 0);

  /* Wait for Device to ready */
  slcnand_wait_ready(dev);

  /* Issue Serial Input Command */
  slcnand_cmd(dev, LPCNAND_CMD_PAGE_WRITE1);

  /* Write Block & Page address */
  slcnand_write_addr(dev, 0, sector);

  /* Set transfer count */
  dev->regs->slc_tc = dev->page_size + dev->spare_size;

  /* Clear ECC*/
  dev->regs->slc_ctrl |= SLCCTRL_ECC_CLEAR;

  /* Start DMA */
  dev->regs->slc_ctrl |= SLCCTRL_DMA_START;

  /* Wait for DMA to Complete Transfer */
  slcnand_start_dma(dev, 0, config);
  stat = slcnand_wait_dma(dev);

  /* Flush out the ECC buffer */
  cp15_force_cache_coherence((void *) dev->ecc,
    (void *)((UNS_8 *) dev->ecc + sizeof(dev->ecc) - 1));

  slcnand_ecc_to_buffer(spare, (unsigned long *)dev->ecc, dev->page_size / 256);

  /* Flush the spare area buffer */
  cp15_force_cache_coherence((void *)spare,
    (void *)((UNS_8 *)spare + dev->spare_size - 1));
  slcnand_start_dma(dev, dev->page_size / 128, config);

  /* Wait for ECC buffer to be transfered */
  stat = slcnand_wait_dma(dev);

  /* Issue Page Program Command to Write Data */
  slcnand_cmd(dev, LPCNAND_CMD_PAGE_WRITE2);

  /* Wait for Device to ready */
  slcnand_wait_ready(dev);

  /* Start DMA */
  dev->regs->slc_ctrl &= ~SLCCTRL_DMA_START;

  /* Disable DMA and Chipselect */
  dev->regs->slc_cfg &= ~(SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
                          SLCCFG_DMA_ECC);
  dev->regs->slc_cfg &= ~SLCCFG_CE_LOW;
  if (stat == _ERROR)
    return _ERROR;

  return slcnand_status(dev);
}


/***********************************************************************
 *
 * Function: slcnand_open
 *
 * Purpose: Open the SLC NAND controller
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a SLC NAND controller peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a SLC NAND controller config structure or
 *          NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slcnand_open(void *ipbase, INT_32 arg)
{
  struct nand_devinfo_list *devlist = (struct nand_devinfo_list *) arg;
  SLCNAND_DEV_T *dev ;
  struct nand_ids *ids;

  /* Handle multiple instance here */
  if (ipbase == SLCNAND)
  {
    dev = &slcnand_device[0];
  }
  else
    return 0;

  if (dev->initialized)
    return (INT_32)dev;

  /* Initialize DMA and clean device structure */
  dma_init();

  /* Turn on clock */
  clkpwr_setup_nand_ctrlr(1, 0, 0);

  /* Enable SLC NAND clock */
  CLKPWR->clkpwr_nand_clk_ctrl |= CLKPWR_NANDCLK_SLCCLK_EN;

  dev->regs = ipbase;
  dev->regs->slc_ctrl = SLCCTRL_SW_RESET | SLCCTRL_ECC_CLEAR;
  dev->regs->slc_tac = (SLCTAC_WDR(15) |
                        SLCTAC_WWIDTH(15) |
                        SLCTAC_WHOLD(15) |
                        SLCTAC_WSETUP(15) |
                        SLCTAC_RDR(15) |
                        SLCTAC_RWIDTH(15) |
                        SLCTAC_RHOLD(15) |
                        SLCTAC_RSETUP(15));
  /* Reset Device */
  slcnand_cmd(dev, NAND_CMD_RESET);
  slcnand_wait_ready(dev);

  ids = slcnand_identify_device(dev, devlist);
  if (!ids)
  {
    /* No NAND device found */
    return 0;
  }

  /**
   * Initialize the device data structure
   **/

  /* Get a DMA channel */
  dev->dmach = dma_alloc_channel(-1, (PFV) slcnand_dma_interrupt);

  if (dev->dmach < 0)
  {
    return _ERROR;
  }

  if (ids->page_size != 2048 && ids->page_size != 512)
    return _ERROR;

  dev->page_size       = ids->page_size;
  dev->block_size      = ids->pages_per_block * ids->page_size;
  dev->chip_size       = ids->total_blocks * dev->block_size;
  dev->spare_size      = ids->page_size / 32;
  dev->pages_per_block = ids->pages_per_block;
  dev->block_count     = ids->total_blocks;
  dev->large_page      = ids->page_size == 2048;
  dev->initialized     = 1;
  dev->v2p             = devlist->virt_to_phy ? devlist->virt_to_phy : slcnand_v2p;

  if (ids->timing.slc)
    dev->regs->slc_tac = ids->timing.slc;

  /* Interrupts disabled and cleared */
  dev->regs->slc_ien = 0;
  dev->regs->slc_icr = (SLCSTAT_INT_TC |
                        SLCSTAT_INT_RDY_EN);
  return (INT_32)dev;
}

/***********************************************************************
 *
 * Function: slcnand_close
 *
 * Purpose: Close the SLC NAND controller
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timers,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS slcnand_close(INT_32 devid)
{
  STATUS status = _ERROR;
  SLCNAND_DEV_T *dev = (SLCNAND_DEV_T *) devid;

  if (!dev || !dev->initialized)
    return status;

  dma_free_channel(dev->dmach);

  /* Turn off clock */
  clkpwr_setup_nand_ctrlr(0, 0, 0);

  /* Disable SLC NAND clock */
  CLKPWR->clkpwr_nand_clk_ctrl &= ~CLKPWR_NANDCLK_SLCCLK_EN;
  dev->initialized = 0;

  return status;
}

/***********************************************************************
 *
 * Function: slcnand_is_block_bad
 *
 * Purpose: Reads the status of NAND operation like erase / program
 *
 * Processing:
 *     See function.
 *
 * Parameters: devid -> NAND Device data structure as integer
 *             block -> block number
 *
 * Outputs: None
 *
 * Returns:  -1 -> Invalid device handle
 *           -2 -> Page read error
 *           0  -> Block is a Good block
 *           1  -> Block is a bad block
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slcnand_is_block_bad(INT_32 devid, INT_32 block)
{
  INT_32 offset;
  SLCNAND_DEV_T *dev = slcnand_dev(devid);

  if (!dev)
    return -1;

  if (slcnand_read_sector(devid, block * dev->pages_per_block, 0, dev->spare_buff) == _ERROR)
    return -2;

  offset = dev->large_page ? NAND_LB_BADBLOCK_OFFS : NAND_SB_BADBLOCK_OFFS;

  return dev->spare_buff[offset] != NAND_GOOD_BLOCK_MARKER;
}

/***********************************************************************
 *
 * Function: slcnand_mark_block_bad
 *
 * Purpose: Reads the status of NAND operation like erase / program
 *
 * Processing:
 *     See function.
 *
 * Parameters: devid -> NAND Device data structure as integer
 *             block -> block number
 *
 * Outputs: None
 *
 * Returns:  _ERROR  -> problem in OOB write
 *           _NO_ERROR  -> operation successful
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slcnand_mark_block_bad(INT_32 devid, INT_32 block)
{
  INT_32 offset;
  SLCNAND_DEV_T *dev = slcnand_dev(devid);

  if (!dev)
    return -1;

  offset = dev->large_page ? NAND_LB_BADBLOCK_OFFS : NAND_SB_BADBLOCK_OFFS;

  memset(dev->spare_buff, 0xFF, 64);

  dev->spare_buff[offset] = 0xFE;
  return slcnand_write_sector(devid, block * dev->pages_per_block, 0, dev->spare_buff);
}

/***********************************************************************
 *
 * Function: slcnand_get_geom
 *
 * Purpose: Fills in the LPC NAND geometry structure
 *
 * Processing:
 *     See function.
 *
 * Parameters: devid -> NAND Device data structure as integer
 *             geom -> (OUT) pointer to geometry structure
 *
 * Outputs: None
 *
 * Returns:  0 -> No Error
 *           -1  -> Error probably device not initialized yet
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slcnand_get_geom(INT_32 devid, NAND_GEOM_T *geom)
{
  SLCNAND_DEV_T *dev = slcnand_dev(devid);
  int addrcyc = 3;

  if (!dev)
    return -1;

  geom->num_blocks = dev->block_count;
  geom->pages_per_block = dev->pages_per_block;
  geom->data_bytes_per_page = dev->page_size;
  geom->spare_bytes_per_page = dev->spare_size;
  /* For large page one more column address */
  if (dev->large_page)
	  addrcyc ++;
  /* For small page devices of size > 32 MiB
   * and large page devices of size > 128 MiB
   * We need one more address cycle */
  if ((!dev->large_page && dev->chip_size > (32 << 20)) ||
      (dev->large_page && dev->chip_size > (128 << 20)))
	  addrcyc ++;
  geom->address_cycles = addrcyc;
  return 0;
}

/***********************************************************************
 *
 * Function: slcnand_get_block
 *
 * Purpose: Get the block number corresponding to a sector
 *
 * Processing:
 *     See function.
 *
 * Parameters: devid -> NAND Device data structure as integer
 *             sector -> Sector number
 *
 * Outputs: None
 *
 * Returns:  >= 0 -> Block number corresponding to the sector
 *           -1  -> Error probably device not initialized yet
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slcnand_get_block(INT_32 devid, INT_32 sector)
{
  SLCNAND_DEV_T *dev = slcnand_dev(devid);

  if (!dev)
    return -1;
  return sector / dev->pages_per_block;
}

/***********************************************************************
 *
 * Function: slcnand_get_page
 *
 * Purpose: Get the page number corresponding to a sector
 *
 * Processing:
 *     See function.
 *
 * Parameters: devid -> NAND Device data structure as integer
 *             sector -> Sector number
 *
 * Outputs: None
 *
 * Returns:  >= 0 -> Page number corresponding to the sector
 *           -1  -> Error probably device not initialized yet
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slcnand_get_page(INT_32 devid, INT_32 sector)
{
  SLCNAND_DEV_T *dev = slcnand_dev(devid);

  if (!dev)
    return -1;
  return sector - (dev->pages_per_block * (sector / dev->pages_per_block));
}
