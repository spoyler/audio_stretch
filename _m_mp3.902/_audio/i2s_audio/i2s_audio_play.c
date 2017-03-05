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

#include "lpc313x_dma_driver.h"
#include "lpc313x_i2s_clock_driver.h"
#include "lpc31xx_vmem_driver.h"


#if 0 //для timer1
#include <NXP/iolpc3131.h>
#else
#include "lpc313x_i2s_driver.h"
#endif

#include "efs.h"



#define   TIMER1_IN_FREQ       6e+6
#define   TIMER1_TICK_MAX      1000   //in miliseconds
#define   TIMER1_TICK_MIN      20     //in miliseconds

#define   TMR1TICK(tick)        ((TIMER1_IN_FREQ/1000)*tick)/256 //Convert miliseconds in Tmr ticks

/** external functions **/
extern void InitSDRAM(void);
extern void Dly_us(UNS_32 Dly);

/** external data **/
UNS_32 Tmr1Tick;
/** internal functions **/
/*************************************************************************
 * Function Name: Timer1IntrHandler
 * Parameters: None
 *
 * Return: None
 *
 * Description: Timer 1 Interrupt service routine. Reload Timer
 *              with Tmr1Tick and toggle the GPIO2_BOOT LED
 *
 *************************************************************************/
#if 0
void Timer1IntrHandler (void)
{

  Timer1Load = Tmr1Tick;

  Timer1Clear = 0;              // clear timer interrupt

  IOCONF_GPIO_M0 ^= (1<<11);
}
#endif
/***********************************************************************
 *
 * Function: fill_buff
 *
 * Purpose: Fills 1/2 of the audio DMA buffer
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *    buff: Pointer to DMA buffer half to fill
 *
 * Outputs: None
 *
 * Returns: Returns 1 when there is no more DMA data to fill
 *
 * Notes: None
 *
 **********************************************************************/
static EmbeddedFile  *_pfile;

INT_32 fill_buff(UNS_32 *buff)
{
  UNS_16 *p16 = (UNS_16 *) buff;
  INT_32 idx, done = 0;
  unsigned short _inbuff;

  for (idx = 0; idx < (AUDIOBUFFSIZE / (2*sizeof(UNS_32))); idx++)
  {
    
    if(file_read(_pfile, 2, (unsigned char *)&_inbuff) == 2)
    {
        *p16++ = _inbuff; //left ch
        *p16++ = _inbuff; //right ch
    }
  }
  lpc31xx_flush_dcache_range((UNS_32 *)buff, (UNS_32 *)((UNS_32)buff + AUDIOBUFFSIZE/2));

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
void dma_callback(INT_32 ch, DMA_IRQ_TYPE_T itype, void *pdmaregs)
{
  INT_32 done = 0;
  UNS_32 irqmask = dmaregs->irq_status_clear & (0x3 << dmach);
#if 1
  if (itype == DMA_IRQ_HALFWAY)
  {
    /* Fill first half of buffer */
    done = fill_buff(&dma_buff[0]);
    dmadoneflag = done;
  }
  
  if (itype == DMA_IRQ_FINISHED)
  {
    /* 2nd half */
    done = fill_buff(&dma_buff[(AUDIOBUFFSIZE / sizeof(UNS_32)) / 2]);
    dmadoneflag = done;
  }
#endif
  dmaregs->irq_status_clear = irqmask;
  dmadoneflag = done;
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
 * Returns: Always returns 1
 *
 * Notes: None
 *
 **********************************************************************/




int _play(EmbeddedFile  *_file)
{
  INT_32 i2sTxdev;
//	I2C_SETUP_T i2c_setup[2];
  int rateidx, ret = _ERROR;

  _pfile = _file;
  
  
#if 0 //time
  Tmr1Tick = TMR1TICK(100);
    /*Enable Timer 1 Clock*/
  //CGU_Run_Clock(TIMER1_PCLK);
  /*Init Timer 1*/
  Timer1Ctrl_bit.Enable = 0;    // disable counting
  Timer1Ctrl_bit.Mode = 1;      // reload mode
  Timer1Ctrl_bit.PreScale = 2;  // prescaler = CGU clock rate / 256
  Timer1Load = Tmr1Tick;
 // set timer 0 period
  Timer1Clear = 0;              // clear timer pending interrupt
  /*Install Interrupt Service Routine,
    Priority 3*/
  int_install_irq_handler(IRQ_TIMER1, (PFV) Timer1IntrHandler);
  /*Enable Timer 1 interrupt*/
  int_enable(IRQ_TIMER1);
  
    /*Start Timer 1*/
  Timer1Ctrl_bit.Enable = 1;
#endif  
  
  
  

  

  /* open the I2S TX1 device  */
  //i2sTxdev = i2s_open(I2S_TX1, (PFV) NULL);

  /* set I2S format for I2S TX - default */
  //i2s_ioctl(i2sTxdev, I2S_FORMAT_SETTINGS, I2S);

  /* Setup DMA - a single circular buffer is used for audio. This buffer
     is 2KBytes in size and is seperated into 2 1K halves. The DMA will
     generate an interrupt when it's halfway and completely done with a
     transfer. These interrupts are used to refill the unused half of
     the buffer with new audio data. */
  dma_init();
  dmaregs = dma_get_base();
  dmach = dma_alloc_channel(0, dma_callback, (void *)dmaregs);
  
  //lpc313x_set_fsmult_rate(256, CLK_USE_256FS);
  //lpc313x_main_clk_rate(44100);
  //lpc313x_chan_clk_enable(CLK_TX_1, 44100,(44100 * 32));

    /* DMA size is number of audio samples times 2 channels times
       16-bits per sample */

    /* Pre-fill audio DMA buffers */
  fill_buff(&dma_buff[0]);
  fill_buff(&dma_buff[(AUDIOBUFFSIZE / sizeof(UNS_32)) / 2]);

    /* Setup DMA channel and circular buffer */
  dmaregs->channel[dmach].source = (UNS_32)lpc31xx_va_to_pa(dma_buff);//(UNS_32)dma_buff;
  dmaregs->channel[dmach].destination = 0x16000160;
  dmaregs->channel[dmach].length = (AUDIOBUFFSIZE / sizeof(UNS_32)) - 1;
#if 1
  dmaregs->channel[dmach].config = (DMA_CFG_CIRC_BUF | DMA_CFG_TX_WORD |
                                      DMA_CFG_RD_SLV_NR(0/*13*/) | DMA_CFG_WR_SLV_NR(9));
#else
  dmaregs->channel[dmach].config = DMA_CFG_CIRC_BUF;  
#endif

    /* enable DMA channel interrupts and start DMA */
  dmaregs->irq_mask &= ~(0x3 << (2 * dmach));
  dmadoneflag = 0;

  
  //dmaregs->channel[dmach].enable =1;
  while(!dmaregs->channel[dmach].enable)
  {
    dmaregs->alt_enable = 1;
  } 
  
  
  
    /* Let DMA handle audio */
  while (1);
    

/* Disable channel */
  dmaregs->irq_mask |= (0x3 << (2 * dmach));
  dmaregs->channel[dmach].enable = 0;
  dma_free_channel(dmach);
  lpc313x_main_clk_rate(0);
  lpc313x_chan_clk_enable(CLK_TX_1, 0, 0);

  return ret;
}
