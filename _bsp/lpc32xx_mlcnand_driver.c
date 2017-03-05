/***********************************************************************
 * $Id:: lpc32xx_mlcnand_driver.c 8093 2011-09-14 16:06:29Z ing03005   $
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

#include <string.h>
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_dma_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc_nandflash_params.h"
#include "lpc32xx_mlcnand_driver.h"
#include "lpc_arm922t_cp15_driver.h"

/***********************************************************************
 * MLC NAND controller driver package data
***********************************************************************/

typedef struct mlcnand_dev
{
  UNS_8  data_buff[2048];
  UNS_8  spare_buff[64];
  DMAC_LL_T dmalist[17];
  INT_32 dmach;
  MLCNAND_REGS_T *regs;
  INT_32 initialized;
  INT_32 page_size;
  INT_32 block_size;
  INT_32 chip_size;
  INT_32 spare_size;
  INT_32 pages_per_block;
  INT_32 block_count;
  INT_32 large_page;
  UNS_32(*v2p)(void *);
}MLCNAND_DEV_T;

/* For now we support only one device */
static /*DMA_BUFFER*/ MLCNAND_DEV_T mlcnand_device[1];

static volatile INT_32 dma_xfer_status = 0;

/***********************************************************************
 * MLC NAND controller driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: mlcnand_cmd
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
static void mlcnand_cmd(const MLCNAND_DEV_T *dev, UNS_32 cmd)
{
  dev->regs->mlc_cmd = cmd;
}

/***********************************************************************
 *
 * Function: mlcnand_wait_ready
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
static void mlcnand_wait_ready(const MLCNAND_DEV_T *dev)
{
  /* Wait for MLC NAND ready */
  while (!(dev->regs->mlc_isr & MLC_DEV_RDY_STS));
}

/***********************************************************************
 *
 * Function: mlcnand_wait_controller
 *
 * Purpose: Busy wait on MLCNAND controller ready
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
static void mlcnand_wait_controller(const MLCNAND_DEV_T *dev)
{
  /* Wait for MLC NAND ready */
  while (!(dev->regs->mlc_isr & MLC_CNTRLLR_RDY_STS));
}

/***********************************************************************
 *
 * Function: mlcnand_status
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
static STATUS mlcnand_status(const MLCNAND_DEV_T *dev)
{
  STATUS status = _NO_ERROR;

  /* Status read (1) command */
  mlcnand_cmd(dev, LPCNAND_CMD_STATUS);
  mlcnand_wait_ready(dev);

  /* Operation status */
  if (*(volatile UNS_8 *)dev->regs->mlc_data & NAND_FLASH_FAILED)
  {
    status = _ERROR;
  }
  return status;
}


/***********************************************************************
 *
 * Function: mlcnand_addr
 *
 * Purpose: Write address to the address register of NAND controller
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
static void mlcnand_addr(const MLCNAND_DEV_T *dev, UNS_32 addr)
{
  dev->regs->mlc_addr = addr;
}

/***********************************************************************
 *
 * Function: mlcnand_v2p
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
static UNS_32 mlcnand_v2p(void *vaddr)
{
  return (UNS_32) vaddr;
}

/***********************************************************************
 *
 * Function: mlcnand_dma_interrupt
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
static void mlcnand_dma_interrupt(int error)
{
  if (!error)
    dma_xfer_status = 1;
  else
    dma_xfer_status = -1;
}

/***********************************************************************
 *
 * Function: mlcnand_wait_dma
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
static STATUS mlcnand_wait_dma(const MLCNAND_DEV_T * dev)
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
 * Function: mlcnand_dev
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
 *          Pointer to MLCNAND_DEV_T on success
 *
 * Notes: None
 *
 **********************************************************************/
static MLCNAND_DEV_T *mlcnand_dev(INT_32 devid)
{
  MLCNAND_DEV_T *dev;
  if (!devid)
    return 0;

  dev = (MLCNAND_DEV_T *) devid;
  if (!dev->initialized)
    return 0;
  return (MLCNAND_DEV_T *)devid;
}

/***********************************************************************
 *
 * Function: mlcnand_read_id
 *
 * Purpose: Read MLC NAND flash id
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as slc was not
 *     previously opened. Otherwise, read MLC NAND flash id.
 *
 * Parameters:
 *     dev:     (IN) Pointer to MLC NAND controller config structure
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
static STATUS mlcnand_read_id(
    const MLCNAND_DEV_T *dev, /* IN: NAND Device structure */
    UNS_8 *man_id,            /* OUT: NAND manufacture ID */
    UNS_8 *dev_id)            /* OUT: NAND Device ID */
{
  UNS_32 tmp;

  if (!dev->regs)
    return _ERROR;


  mlcnand_cmd(dev, NAND_CMD_READID);
  mlcnand_addr(dev, 0);
  tmp = dev->regs->mlc_data[0];
  *man_id = tmp & 0xFF; tmp >>= 8;
  *dev_id = tmp & 0xFF;

  return _NO_ERROR;
}

/***********************************************************************
 *
 * Function: mlcnand_identify_device
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
static struct nand_ids * mlcnand_identify_device(
    const MLCNAND_DEV_T *dev,
    const struct nand_devinfo_list *dlst)
{
  int i;
  UNS_8 mfid, devid;
  if (mlcnand_read_id(dev, &mfid, &devid) == _ERROR)
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
 * Function: mlcnand_write_addr
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
static void mlcnand_write_addr(
    const MLCNAND_DEV_T *dev,
    INT_32 column,
    UNS_32 sector)
{
  /* Write column address if given */
  if (column != -1)
  {
    mlcnand_addr(dev, column & 0xFF);
    if (dev->large_page)
      mlcnand_addr(dev, ((UNS_32)column >> 8) & 0xFF);
  }

  /* Write the page address */
  mlcnand_addr(dev, sector & 0xFF);
  mlcnand_addr(dev, (sector >> 8) & 0xFF);

  /* For small page devices of size > 32 MiB
   * and large page devices of size > 128 MiB
   * We need one more address cycle */
  if ((!dev->large_page && dev->chip_size > (32 << 20)) ||
      (dev->large_page && dev->chip_size > (128 << 20)))
    mlcnand_addr(dev, (sector >> 16) & 0xFF);
}

/***********************************************************************
 *
 * Function: mlcnand_start_dma
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
static void mlcnand_start_dma(
    MLCNAND_DEV_T *dev,
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
 * Function: mlcnand_prepare_dma
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
static STATUS mlcnand_prepare_dma(
    MLCNAND_DEV_T *dev,
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

  ctrl = ( DMAC_CHAN_SRC_BURST_1 |
           DMAC_CHAN_DEST_BURST_1 |
           DMAC_CHAN_SRC_WIDTH_32 |
           DMAC_CHAN_DEST_WIDTH_32 |
           DMAC_CHAN_DEST_AHB1 |
           (read ? DMAC_CHAN_DEST_AUTOINC : DMAC_CHAN_SRC_AUTOINC));

  dmalst = dev->dmalist;

  for (i = 0; i < dev->page_size / 512; i++, dmalst++)
  {
    /* Prepare DATA descriptor */
    dmalst->dma_src = dev->v2p(read ? (void *) dev->regs->mlc_buff :
                               &buff[i * 512]);
    dmalst->dma_dest = dev->v2p(!read ? (void *) dev->regs->mlc_buff :
                                &buff[i * 512]);
    dmalst->next_ctrl = 128 | ctrl; /* RD/WR 128 words per descriptor */
    dmalst->next_lli = dev->v2p(dmalst + 1);
    dmalst ++;

    /* Prepare Spare descriptor */
    dmalst->dma_src = dev->v2p(read ? (void *) dev->regs->mlc_buff :
                               &oob[i * 16]);
    dmalst->dma_dest = dev->v2p(!read ? (void *) dev->regs->mlc_buff :
                                &oob[i * 16]);
    dmalst->next_lli = 0;
    dmalst->next_ctrl = 4 | ctrl | DMAC_CHAN_INT_TC_EN;
  }

  /* Flush out the cached descriptors */
  cp15_force_cache_coherence((void *) dev->dmalist,
    (void*)((UNS_8 *)dev->dmalist + sizeof(dev->dmalist) - 1));

  return status;
}

/***********************************************************************
 * MLC NAND controller driver public functions
 **********************************************************************/
/***********************************************************************
 *
 * Function: mlcnand_erase_block
 *
 * Purpose: Erase MLC NAND flash block
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as slc was not
 *     previously opened. Otherwise, erase MLC NAND flash block.
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *     block_num: Block to be erased
 *
 * Outputs: None
 *
 * Returns: The status of block erase operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mlcnand_erase_block(INT_32 devid, UNS_32 block_num)
{
  STATUS status = _ERROR;
  MLCNAND_DEV_T *dev = mlcnand_dev(devid);

  if (!dev->initialized)
    return status;

  mlcnand_wait_ready(dev);
  mlcnand_cmd(dev, NAND_CMD_ERASE1ST);
  mlcnand_write_addr(dev, -1, block_num * dev->pages_per_block);
  mlcnand_cmd(dev, NAND_CMD_ERASE2ND);
  mlcnand_wait_ready(dev);
  return mlcnand_status(dev);
}


/***********************************************************************
 *
 * Function: mlcnand_read_sector
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
STATUS mlcnand_read_sector(
    INT_32 devid,
    INT_32 sector,
    UNS_8 *data,
    UNS_8 *spare)
{
  int i;
  STATUS stat = _NO_ERROR;
  MLCNAND_DEV_T *dev = mlcnand_dev(devid);
  UNS_32 config = DMAC_CHAN_ITC |
                  DMAC_CHAN_IE |
                  DMAC_CHAN_FLOW_D_M2M |
                  DMAC_DEST_PERIP(0) |
                  DMAC_SRC_PERIP(0) |
                  DMAC_CHAN_ENABLE;

  if (!dev)
    return _ERROR;
  
//16 bits mode  
  dev->regs->mlc_icr = (MLC_LARGE_BLK_ENABLE | MLC_ADDR4_ENABLE | MLC_DATA16_ENABLE);

  /* Enable Chip select */
  dev->regs->mlc_ceh = MLC_NORMAL_NCE;

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

  mlcnand_wait_ready(dev);

  /* Issue page read1 command */
  mlcnand_cmd(dev, LPCNAND_CMD_PAGE_READ1);

  /* Write address */
  mlcnand_write_addr(dev, 0, sector);
  mlcnand_wait_ready(dev);

  mlcnand_cmd(dev, LPCNAND_CMD_PAGE_READ2);

  /* Wait for ready */
  mlcnand_wait_ready(dev);

  /* Prepare DMA descriptors and do the read */
  mlcnand_prepare_dma(dev, data, spare, 1);

  /* Start reading 512 Byte data blocks */
  for (i = 0; i < dev->page_size/512; i++)
  {
    /* ECC Auto Decode */
    dev->regs->mlc_autodec_dec = 0;
    mlcnand_wait_controller(dev);

    /* Wait for ECC ready */
    while ((dev->regs->mlc_isr & MLC_ECC_RDY_STS) == 0);

    if (dev->regs->mlc_isr & (MLC_DECODE_ERR_DETECT_STS |
        MLC_DECODE_FAIL_STS))
    {
        stat = _ERROR;
    }

	/* Start the DMA */
    mlcnand_start_dma(dev, i * 2, config);

    /* Wait for DMA to Complete */
    if (mlcnand_wait_dma(dev) == _ERROR)
    {
      stat = _ERROR;
      break;
    }
  }

  /* Deassert nCE */
  dev->regs->mlc_ceh = 0;

  /* Wait for ready */
  mlcnand_wait_ready(dev);

  /* Invalidate the buffers */
  cp15_force_cache_coherence((void *)data,
    (void *)((UNS_8 *)data + dev->page_size - 1));
  cp15_force_cache_coherence((void *)spare,
    (void *)((UNS_8 *)spare + dev->spare_size - 1));

  /* Check if we have any un_correctable read errors */
  return stat;
}

/***********************************************************************
 *
 * Function: mlcnand_write_sector
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
STATUS mlcnand_write_sector(
    INT_32 devid,
    INT_32 sector,
    UNS_8 *data,
    UNS_8 *spare)
{
  int i;
  STATUS stat = _NO_ERROR;
  MLCNAND_DEV_T *dev = mlcnand_dev(devid);
  UNS_32 config = DMAC_CHAN_ITC |
                  DMAC_CHAN_IE |
                  DMAC_CHAN_FLOW_D_M2M |
                  DMAC_DEST_PERIP(0) |
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
  cp15_force_cache_coherence((void *)spare,
    (void *)((UNS_8 *)spare + dev->spare_size - 1));

  /* Force nCE for the entire cycle */
  dev->regs->mlc_ceh = MLC_NORMAL_NCE;

  /* Issue Serial Input Command */
  mlcnand_cmd(dev, LPCNAND_CMD_PAGE_WRITE1);

  /* Write Block & Page address */
  mlcnand_write_addr(dev, 0, sector);
  mlcnand_wait_ready(dev);

  /* Prepare DMA descriptors */
  mlcnand_prepare_dma(dev, data, spare, 0);

  for (i = 0; i < dev->page_size/512; i++)
  {
    /* ECC Encode */
    dev->regs->mlc_enc_ecc = 0;

    /* Wait for DMA to Complete Transfer */
    mlcnand_start_dma(dev, i * 2, config);
    if (mlcnand_wait_dma(dev) == _ERROR)
    {
      stat = _ERROR;
      break;
    }

    /* ECC Auto Encode */
    dev->regs->mlc_autoenc_enc = 0;
    /* Wait for Device to ready */
    mlcnand_wait_controller(dev);
  }

  /* Issue Page Program Command to Write Data */
  mlcnand_cmd(dev, LPCNAND_CMD_PAGE_WRITE2);

  /* Wait for Device to ready */
  mlcnand_wait_ready(dev);

  if (stat == _ERROR)
    return _ERROR;

  return mlcnand_status(dev);
}

/***********************************************************************
 *
 * Function: mlcnand_open
 *
 * Purpose: Open the MLC NAND controller
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a MLC NAND controller peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a MLC NAND controller config structure or
 *          NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mlcnand_open(void *ipbase, INT_32 arg)
{
  struct nand_devinfo_list *devlist = (struct nand_devinfo_list *) arg;
  MLCNAND_DEV_T *dev ;
  struct nand_ids *ids;

  /* Handle multiple instance here */
  if (ipbase == MLCNAND)
  {
    dev = &mlcnand_device[0];
  }
  else
    return 0;

  if (dev->initialized)
    return (INT_32)dev;

  /* Initialize DMA and clean device structure */
  dma_init();
  memset(dev, 0, sizeof(MLCNAND_DEV_T));

  /* Turn on clock */
  clkpwr_setup_nand_ctrlr(0, 0, 0);
  CLKPWR->clkpwr_nand_clk_ctrl |= CLKPWR_NANDCLK_MLCCLK_EN;

  dev->regs = ipbase;
  dev->regs->mlc_lock_pr = MLC_UNLOCK_REG_VALUE;
  dev->regs->mlc_icr = (MLC_LARGE_BLK_ENABLE | MLC_ADDR4_ENABLE);

  /* Setup MLC timing to a very conservative (slowest) timing */
  dev->regs->mlc_lock_pr = MLC_UNLOCK_REG_VALUE;
  dev->regs->mlc_time = (MLC_LOAD_TCEA(15) |
    MLC_LOAD_TWBTRB(15) |
    MLC_LOAD_TRHZ(15) |
    MLC_LOAD_TREH(15) |
    MLC_LOAD_TRP(15) |
    MLC_LOAD_TWH(15) |
    MLC_LOAD_TWP(15));
  dev->regs->mlc_irq_mr = 0;
  /* Normal chip enable operation */
  dev->regs->mlc_ceh = MLC_NORMAL_NCE;

  /* Reset Device */
  mlcnand_cmd(dev, LPCNAND_CMD_RESET);
  mlcnand_wait_ready(dev);

  /* Reset buffer pointer */
  dev->regs->mlc_rubp = 0x1;

  ids = mlcnand_identify_device(dev, devlist);
  if (!ids)
  {
    /* No NAND device found */
    return 0;
  }

  /**
   * Initialize the device data structure
   **/

  /* Get a DMA channel */
  dev->dmach = dma_alloc_channel(-1, (PFV) mlcnand_dma_interrupt);

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
  dev->v2p             = devlist->virt_to_phy ? devlist->virt_to_phy : mlcnand_v2p;

  if (ids->timing.mlc)
    dev->regs->mlc_time = ids->timing.mlc;

  return (INT_32)dev;
}

/***********************************************************************
 *
 * Function: mlcnand_close
 *
 * Purpose: Close the MLC NAND controller
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timers,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mlcnand_close(INT_32 devid)
{
  STATUS status = _ERROR;
  MLCNAND_DEV_T *dev = (MLCNAND_DEV_T *) devid;

  if (!dev || !dev->initialized)
    return status;

  dma_free_channel(dev->dmach);

  /* Turn off clock */
  clkpwr_setup_nand_ctrlr(0, 0, 0);

  /* Disable MLC NAND clock */
  CLKPWR->clkpwr_nand_clk_ctrl &= ~CLKPWR_NANDCLK_MLCCLK_EN;
  dev->initialized = 0;

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_is_block_bad
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
INT_32 mlcnand_is_block_bad(INT_32 devid, INT_32 block)
{
  INT_32 offset;
  MLCNAND_DEV_T *dev = mlcnand_dev(devid);

  if (!dev)
    return -1;

  if (mlcnand_read_sector(devid, block * dev->pages_per_block, 0, dev->spare_buff) == _ERROR)
    return -2;

  offset = dev->large_page ? NAND_LB_BADBLOCK_OFFS : NAND_SB_BADBLOCK_OFFS;

  return dev->spare_buff[offset] != NAND_GOOD_BLOCK_MARKER;
}

/***********************************************************************
 *
 * Function: mlcnand_mark_block_bad
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
INT_32 mlcnand_mark_block_bad(INT_32 devid, INT_32 block)
{
  INT_32 offset;
  MLCNAND_DEV_T *dev = mlcnand_dev(devid);

  if (!dev)
    return -1;

  offset = dev->large_page ? NAND_LB_BADBLOCK_OFFS : NAND_SB_BADBLOCK_OFFS;

  memset(dev->spare_buff, 0xFF, 64);

  dev->spare_buff[offset] = 0xFE;
  return mlcnand_write_sector(devid, block * dev->pages_per_block, 0, dev->spare_buff);
}

/***********************************************************************
 *
 * Function: mlcnand_get_geom
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
INT_32 mlcnand_get_geom(INT_32 devid, NAND_GEOM_T *geom)
{
  MLCNAND_DEV_T *dev = mlcnand_dev(devid);
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
 * Function: mlcnand_get_block
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
INT_32 mlcnand_get_block(INT_32 devid, INT_32 sector)
{
  MLCNAND_DEV_T *dev = mlcnand_dev(devid);

  if (!dev)
    return -1;
  return sector / dev->pages_per_block;
}

/***********************************************************************
 *
 * Function: mlcnand_get_page
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
INT_32 mlcnand_get_page(INT_32 devid, INT_32 sector)
{
  MLCNAND_DEV_T *dev = mlcnand_dev(devid);

  if (!dev)
    return -1;
  return sector - (dev->pages_per_block * (sector / dev->pages_per_block));
}
