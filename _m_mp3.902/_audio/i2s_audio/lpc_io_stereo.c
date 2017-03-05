/*--------------------------------------------------------------------------*/
/* 
 * MP3 player DEMO - MPEG queue management functions
 * Copyright (C) 2006 MXP semicontuctor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY and WUTHOUT ANY SUPPORT; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: lpc_io.c 1.0 2006/10/16 09:41:32 rob Exp $
 */
/*--------------------------------------------------------------------------*/
#include  <string.h>
#include  "lpc_io_stereo.h"

#include "lpc313x_dma_driver.h"
#include "lpc313x_i2s_clock_driver.h"
#include "lpc31xx_vmem_driver.h"
#include "lpc313x_cgu_driver.h"


extern unsigned char m_env31_;
extern unsigned char m_env24_;
extern unsigned char _b_s_down;
extern unsigned char _b_s_up;
extern unsigned char _b_vol_down;
extern unsigned char _b_vol_down1;

//когда звучит rem на действие в player 
//lock клавы и задач
extern unsigned char m_mylock;
//уровень громкости
//extern unsigned short _k;
extern unsigned char __k;
//режим необходимости воспр звук ПИК
extern unsigned char m_env9;

//режим необходимости воспр звук ПИК
extern unsigned char m_env100;

#define XTAL_IN		        (12000000)


static decoded_stream_t    DecodedBuff;
static bool                RenderStatus; /* Audio render status  */
                                          /* TRUE = render enabled , FALSE =render disabled */

#define DFIFO_DELETE()     (DecodedBuff.wr_idx = DecodedBuff.rd_idx = 0)
#define IS_DFIFO_FULL()    ((DecodedBuff.wr_idx+1)%STREAM_DECODED_SIZE == DecodedBuff.rd_idx)
#define IS_DFIFO_EMPTY()   (DecodedBuff.wr_idx == DecodedBuff.rd_idx)

#define DFIFO_WRITE(S)     do {                                                                   \
                             DecodedBuff.raw[DecodedBuff.wr_idx] = S;                             \
                             DecodedBuff.wr_idx = ( (DecodedBuff.wr_idx+1)%STREAM_DECODED_SIZE ); \
                           }while(0)

#define DFIFO_READ(S)      do {                                                                   \
                             S = DecodedBuff.raw[DecodedBuff.rd_idx];                             \
                             DecodedBuff.rd_idx = ( (DecodedBuff.rd_idx+1)%STREAM_DECODED_SIZE ); \
                           }while(0)

#define FIFO_LEN()        ( DecodedBuff.wr_idx >= DecodedBuff.rd_idx ? \
                            DecodedBuff.wr_idx - DecodedBuff.rd_idx  : \
                            DecodedBuff.wr_idx + (STREAM_DECODED_SIZE - DecodedBuff.rd_idx) )



#define AUDIOBUFFSIZE (sizeof(UNS_32) * 1024)


static UNS_32 dma_buff[AUDIOBUFFSIZE / sizeof(UNS_32)];
static volatile INT_32 dmadoneflag, dmach, str_rem;
DMAC_REGS_T *dmaregs;
static const UNS_32 test_rates[] =
{
  8000, 11025, 12000, 16000, 22050, 32000, 44100, 48000, 64000,
  88200, 96000
};

unsigned char _max_counter=1;//сколько раз из buffer будем вычитывать sample
                              //нужно для убыстрения воспроиз. внутри фрагмента


//DEBUG
static unsigned char _dbg1;

void _init_render()
{
  DFIFO_DELETE();
  RenderStatus = DISABLED;
}

void enable_audio_render(void)
{
    if (RenderStatus == DISABLED)
    {
      RenderStatus = ENABLED;
    /* enable DMA channel interrupts and start DMA */
      dmaregs->irq_mask &= ~(0x3 << (2 * dmach));      
      dmaregs->channel[dmach].enable  =1;
      
      _dbg1 |= 0x1;
    }
}

void disable_audio_render(void)
{
    if (RenderStatus == ENABLED)
    {
      RenderStatus = DISABLED;
      dmaregs->irq_mask |= (0x3 << (2 * dmach));
      dmaregs->channel[dmach].enable = 0;
      _init_render(); 
      fill_buff(&dma_buff[0]);
      fill_buff(&dma_buff[(AUDIOBUFFSIZE / sizeof(UNS_32)) / 2]); 
      
      _dbg1 = 0x0;
    }
}

void wait_end_of_excerpt(void)
{
    while (!IS_DFIFO_EMPTY());
    disable_audio_render();
    memset(DecodedBuff.raw,0x0,sizeof(DecodedBuff.raw));
    DFIFO_DELETE();
}





void render_sample_block(unsigned short *blk_ch1, unsigned short *blk_ch2, unsigned short int len, unsigned char nch)
{
  unsigned short i;
  unsigned long sample;
static unsigned char _dbg_counter = 0x0;  

#if 1
    if((_dbg1 & 0x1) && !(_dbg1 & 0x2))
    {
      if(_dbg_counter <= 30)
      {
        _dbg_counter++;
        return;
      }
      else
      {
        _dbg_counter=0x0;
        _dbg1 |= 0x2;
      }
    }    
#endif        
  
    for (i=0; i<len; i++)
    {
      while (IS_DFIFO_FULL())
        asm("  NOP  ");
      sample=0;
      if(nch==2)
      {
        sample = (unsigned short)*(blk_ch2+i);
        sample <<= 16;
        sample += (unsigned short)*(blk_ch1+i);
      }  
      else
      {
        sample = (unsigned short)*(blk_ch1+i);
        sample <<= 16;
        sample += (unsigned short)*(blk_ch1+i);
      }
      DFIFO_WRITE(sample);      
    }


    
}

INT_32 fill_buff(UNS_32 *buff)
{
UNS_32 *p32 = (UNS_32 *) buff;
INT_32 idx, done = 0;
UNS_32 sample;  
  

  for (idx = 0; idx < (AUDIOBUFFSIZE / (2*sizeof(UNS_32))); idx++)
  {
    if (!IS_DFIFO_EMPTY())
    {
      DFIFO_READ(sample);
      *p32++ = sample;
    }
    else
      *p32++ = 0x00000000;
  }
  _tci_loop();
  //lpc31xx_flush_dcache_range((UNS_32 *)buff, (UNS_32 *)((UNS_32)buff + AUDIOBUFFSIZE));

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
  
  if (itype == DMA_IRQ_HALFWAY)
  {
    /* Fill first half of buffer */
    done = fill_buff(&dma_buff[0]);

  }
  
  if (itype == DMA_IRQ_FINISHED)
  {
    /* 2nd half */
    done = fill_buff(&dma_buff[(AUDIOBUFFSIZE / sizeof(UNS_32)) / 2]);
    dmadoneflag = done;
  }

  dmaregs->irq_status_clear = irqmask;
}

void _clock_setup(void)
{
  UNS_32 clkin_freq[CGU_FIN_SELECT_MAX];
#if 1
  CGU_HPLL_SETUP_T pll_180 =
	{
        CGU_FIN_SELECT_FFAST, //fin_select;
        770,		//ndec;
        8191,		//mdec;
        98,			//pdec;
        0,			//selr;
        16,			//seli;
        8,			//selp;
        0,			//mode;
        180000000	//freq;
    };
#if 1
  CGU_HPLL_SETUP_T pll0 = {
        CGU_FIN_SELECT_FFAST, //fin_select;
        131,			//ndec;
        29784,		//mdec;
        7,			//pdec;
        0,			//selr;
        8,			//seli;
        31,			//selp;
        0,			//mode;
        406425600	//freq;
    };
#else
  CGU_HPLL_SETUP_T pll0 = {
        CGU_FIN_SELECT_FFAST, //fin_select;
        770,			//ndec;
        8191,		//mdec;
        2,			//pdec;
        0,			//selr;
        16,			//seli;
        8,			//selp;
        0,			//mode;
        44100 * 1024	//freq;
    };  
#endif  
  
#endif
  /* init clock frequencies */
  clkin_freq[0] = XTAL_IN; /* CGU_FIN_SELECT_FFAST */
  clkin_freq[1] = 0; /*	CGU_FIN_SELECT_XT_DAI_BCK0 */
  clkin_freq[2] = 0; /*	CGU_FIN_SELECT_XT_DAI_WS0 */
  clkin_freq[3] = 0; /*	CGU_FIN_SELECT_XT_DAI_BCK1 */
  clkin_freq[4] = 0; /*	CGU_FIN_SELECT_XT_DAI_WS1 */
  clkin_freq[5] = 0; /*	CGU_FIN_SELECT_HPPLL0     */
  clkin_freq[6] = 0; /*	CGU_FIN_SELECT_HPPLL1     */

  /* init CGU driver */
  cgu_init(clkin_freq);
  
#if 1
  cgu_reset_all_clks();
  /* set HPLL0 - Audio PLL to default speed */
  cgu_hpll_config (CGU_HPLL0_ID, &pll0); 
  /* set HPLL1 - main PLL to default speed */
  /* Check if we are running on LPC3141 or LPC3143. If yes use 270/90
  settings. For all other chips use 180/90
  */
  cgu_hpll_config (CGU_HPLL1_ID, &pll_180);
    /* use the default clock tree distribution structure */
  cgu_init_clks(&g_cgu_default_clks);
  

#endif  
}

void __dma_free_channel(void )
{
  dma_free_channel(dmach);
}


void init_timer(void)
{
INT_32 i2sTxdev;
CGU_FDIV_SETUP_T fdiv_cfg;

#if 0
  _clock_setup();
#endif
  
#if 1  
  tlv320aic23_probe(44100); //инит aic23
#endif
  //on/off booster
  _booster(0x01);
  _booster_mux(0x0);
  _booster_silence(0x0);
  
#if 0    
    for(unsigned long _l1=0;_l1<0x01ffffff;_l1++)
      asm("  NOP  ");
#endif  
  * (UNS_32 *) 0x130044dc = 5;
  lpc313x_set_fsmult_rate(256, CLK_USE_256FS | CLK_USE_TXCLKO); 
    
  dma_init();
  dmaregs = dma_get_base();
  dmach = dma_alloc_channel(0, dma_callback, (void *)dmaregs);
  
  //lpc313x_set_fsmult_rate(128, 0);
  //lpc313x_main_clk_rate(22050);
#if 0  
  lpc313x_main_clk_rate(44100);
  lpc313x_chan_clk_enable(CLK_TX_1, 44100,(44100 * 16*2));
#endif  
  _init_render();
  fill_buff(&dma_buff[0]);
  fill_buff(&dma_buff[(AUDIOBUFFSIZE / sizeof(UNS_32)) / 2]);

  /* Setup DMA channel and circular buffer */
  dmaregs->channel[dmach].source = (UNS_32)lpc31xx_va_to_pa(dma_buff);//(UNS_32)dma_buff;
  dmaregs->channel[dmach].destination = 0x16000160;
  dmaregs->channel[dmach].length = (AUDIOBUFFSIZE / sizeof(UNS_32)) - 1;
  dmaregs->channel[dmach].config = (DMA_CFG_CIRC_BUF | DMA_CFG_TX_WORD |
                                      DMA_CFG_RD_SLV_NR(0/*13*/) | DMA_CFG_WR_SLV_NR(9));

  //dmaregs->irq_mask &= ~(0x3 << (2 * dmach));
  //dmaregs->channel[dmach].enable  =1;
  
//DEBUG
  _dbg1 = 0x0;  
}


float _k1=1.0;
unsigned char _b_s_down=0x0;
void set_dac_sample_rate(unsigned int sample_rate)
{
static unsigned char _var=0;
static unsigned char _var3=0x0;
static unsigned long _srate_prev =0;

  if (RenderStatus == DISABLED)
  {
/*
    aic23 после настройки дает 100khz шум-импульс, что порождает
    щелчек в усилке, можно конечно сделать НЧ фильтр аналоговый, но
    мы просто переключим mux усилка с aic23 на приемник
*/    
    //on silence
    //_booster_silence(0x01);
    //mux booster
    //_booster_mux(0x01);  
#if 1 
    if(_srate_prev!=sample_rate)
    {        
      _set_srate(sample_rate);
      if(!((sample_rate == 22050) || (sample_rate == 44100)))
        lpc313x_main_clk_rate(sample_rate);
      lpc313x_chan_clk_enable(CLK_TX_1, sample_rate,(sample_rate * 16*2));
      _set_master();
      _srate_prev = sample_rate;
#if 0    
    for(unsigned long _l1=0;_l1<0x01ffffff;_l1++)
      asm("  NOP  ");
#endif      
    }
#endif  
//вернем mux обратно на aic23     
    //mux booster
    //_booster_mux(0x0);    
    //off silence
    //_booster_silence(0x0); 
    enable_audio_render();
    
  }

#if 0
  if(!m_mylock)
  {
#if 0
//посмотрим не надо ли поперемещаться в фрагменте вперед
//если конечно не rem player    
    if(_forward_frag()==0x01)
    {
      if(_var++ == 1) //слишком быстро меняется скорость перемещения внутри фрагмента
                      //надо бы замедлить        
      {
        if(_max_counter<7)
        _max_counter+=1;
        _var=0;
      }
      
    }
    if(_max_counter>1)
      if(_forward_frag()==0x02)
      {
//перемещение вперед завершено      
        _max_counter=1;
        _var=0;
      }
    
#endif    
  }
#endif  
#if 0
//посмотрим не надо ли менять уровень громкости
    if(_volume_down())
    {
      if(__k<0x7f)
      {
        __k++;
        _aic23_svolume(0x0080+__k);
      }
      else
        m_env9 |= 0x01; //поставим задачу воспр. пик
      
    }
    if(_volume_up())
    {
      if(__k>0x60)
      {
        __k--;
        _aic23_svolume(0x0080+__k);
      }
      else
        m_env9 |= 0x01; //поставим задачу воспр. пик      
      
    }
#endif


#if 1  
//может надо скорость изменить, смотрим    
    _b_s_down = _fast_down();
    if(_b_s_down==0x02)
      m_env31_|=0x01; //надо при нажатии на стоп блокир клаву
    if(_b_s_down==0x0/*0x03*/)
      m_env31_&=~0x01; //отмена блокир клавы
    
    if(_b_s_down==0x02 && !(m_env24_ & 0x01))
      _var3 = 0x01;
    if(_b_vol_down1 && _var3 && (!m_env24_ || (m_env24_ & 0x80)))  
    {
      m_env24_ |= 0x01;
      _var3 = 0x0;
    }
    else
    {
#if 0      
      if(!m_env24_)
      if((_b_s_down==0x01) || (_b_s_down==0x02))
      {
        if(_k1>1.0)
        {
          _k1-=0.1;
          //m_env100 |= 0x01; //поставим задачу измен темп корректора 
        }
        else
          m_env9 |= 0x01; //поставим задачу воспр. пик      
      
      }
      if(_fast_up())
      {
        if(_k1 <1.9)
        {
          _k1+=0.1;
          //m_env100 |= 0x01; //поставим задачу измен темп корректора
        }
        else
          m_env9 |= 0x01; //поставим задачу воспр. пик      
      }
#endif      
      _var3 = 0x0;
    }
#endif  
      
  
}

void _clear_buff()
{
    DFIFO_DELETE();
    RenderStatus = DISABLED;
  
}


void init_IO(void)
{
unsigned char       file_name[50]; 
unsigned char _prev_k;
    

    
#if 1
    //if(!_get_card())
      init_timer();


#else
    sample=0x8000;
    while(1)
    {
      _write_sample(sample);    
    }
#endif    

}
