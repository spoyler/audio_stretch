/***********************************************************************
 * $Id:: i2s_audio_example.c 8087 2011-09-14 04:13:00Z ing02124        $
 *
 * Project: I2S audio driver example
 *
 * Description:
 *     This example uses I2C to initialize the audio CODEC, sets up
 *     correct I2S clocking, and then uses circular mode DMA to play
 *     an audio stream.
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

#include "lpc_types.h"
#include "lpc_irq_fiq.h"

#include "lpc32xx_dma_driver.h"
#include "lpc32xx_i2s_driver.h"
#include "lpc32xx_clkpwr_driver.h"

#define AUDIOBUFFSIZE 1//подправим позжеs
static const INT_32 dma_buff[AUDIOBUFFSIZE]={
  0x80010001,
  /*0x00000000,
  0x00000000,
  0x00000000,
  0x80000001*/
};


INT_32 fill_buff(UNS_32 *buff)
{
  UNS_16 *p16 = (UNS_16 *) buff;
  INT_32 idx, done = 0;
  unsigned short _inbuff;

  for (idx = 0; idx < (AUDIOBUFFSIZE / (2*sizeof(UNS_32))); idx++)
  {
    //_inbuff может содержать sample sine
        *p16++ = _inbuff; //left ch
        *p16++ = _inbuff; //right ch
   
  }
  //dma имеет доступ к некэшируемой области
  //lpc31xx_flush_dcache_range((UNS_32 *)buff, (UNS_32 *)((UNS_32)buff + AUDIOBUFFSIZE/2));

  return done;
}

/***********************************************************************
 *
 * Function: dma_callback
 *
 * Purpose: Audio DMA handler callback
 *
 * Processing:
 *     For each call to this handler, the DMA circuilar buffer half
 *     that isn't active is refilled. Once all data is transferred,
 *     a flag is set to end the program.
 *
 * Parameters:
 *    pdmaregs: Pointer to DMA registers
 *
 * Outputs: None
 *
 * Returns: Always returns 1
 *
 * Notes: None
 *
 **********************************************************************/
void dma_callback(void/*INT_32 ch, DMA_IRQ_TYPE_T itype, void *pdmaregs*/)
{
  asm("nop");
}





void _i2s_entry(void) {
  INT_32 i2s;
  int rateidx, ret = _ERROR;
  I2S_PRMS_T _par0;
  I2S_DMA_PRMS_T _dma_par0;
  
    /* Enable I2S0 clock */
  clkpwr_clk_en_dis(CLKPWR_I2S0_CLK,1);
  
  i2s = i2s_open(I2S0, 0);
  i2s_ioctl(i2s, I2S_DO_RESET, 1);
  i2s_ioctl(i2s, I2S_DO_RESET, 0);
  _par0.i2s_word_width = I2S_WW32;
  _par0.i2s_tx_slave = 1;
  i2s_ioctl(i2s, I2S_SETUP_PARAMS, *((INT_32 *)&_par0));
  i2s_ioctl(i2s, I2S_DO_MUTE, 0);
  
 
  
  dma_init();
  //dmaregs = dma_get_base();
  _dma_par0.dmach = dma_alloc_channel(0, dma_callback); 
  _dma_par0.dir = DMAC_CHAN_FLOW_D_M2P;
  _dma_par0.mem = (INT_32)(&dma_buff[0]);
  _dma_par0.sz = AUDIOBUFFSIZE;
  i2s_dma_init_dev(i2s, (I2S_DMA_PRMS_T *)&_dma_par0);

  i2s_ioctl(i2s, I2Sx_DMA0_TX_EN, 1);
 
#if 0  
    /* Let DMA handle audio */
  while (1)
    asm("nop");
    


  i2s_ioctl(i2s, I2Sx_DMA0_TX_EN, 0);
  dma_free_channel(_dma_par0.dmach);
#endif

  return;
}
