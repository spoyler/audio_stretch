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

#include "lpc_types.h"
//#include "lpc_irq_fiq.h"

#include "lpc32xx_dma_driver.h"
#include "lpc32xx_i2s_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_gpio_driver.h"


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



#define AUDIOBUFFSIZE 1024 //32 bits words


static UNS_32 dma_buff[AUDIOBUFFSIZE];
static volatile INT_32 dmadoneflag, dmach, str_rem;
DMAC_REGS_T *dmaregs;
static const UNS_32 test_rates[] =
{
  8000, 11025, 12000, 16000, 22050, 32000, 44100, 48000, 64000,
  88200, 96000
};

unsigned char _max_counter=1;//сколько раз из buffer будем вычитывать sample
                              //нужно для убыстрения воспроиз. внутри фрагмента

static unsigned char _half_buff;
static INT_32 i2s;
I2S_DMA_PRMS_T _dma_par0;

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
      _half_buff=0;
    /* enable DMA channel interrupts and start DMA */    
      i2s_ioctl(i2s, I2Sx_DMA0_TX_EN, 1);
      
      //_dbg1 |= 0x1;
    }
}

I2S_PRMS_T _par0;
static void __init_dma(void){
    i2s_dma_init_dev(i2s, (I2S_DMA_PRMS_T *)&_dma_par0);
}

void disable_audio_render(void)
{
    if (RenderStatus == ENABLED)
    {
      RenderStatus = DISABLED;
      i2s_ioctl(i2s, I2Sx_DMA0_TX_EN, 0);
      _half_buff=0;
      __init_dma();
      _init_render(); 
      fill_buff(&dma_buff[0]);
      fill_buff(&dma_buff[AUDIOBUFFSIZE/2]); 
      
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

#if 0
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
#if 1      
      while (IS_DFIFO_FULL())
        asm("  NOP  ");
#endif      
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
#if 1      
      DFIFO_WRITE(sample);   
#endif      
    }


    
}

INT_32 fill_buff(UNS_32 *buff)
{
UNS_32 *p32 = (UNS_32 *) buff;
INT_32 idx, done = 0;
UNS_32 sample;  
  

  for (idx = 0; idx < (AUDIOBUFFSIZE/2); idx++)
  {
    if (!IS_DFIFO_EMPTY())
    {
      DFIFO_READ(sample);
      *p32++ = sample;
    }
    else
      *p32++ = 0x00000000;
  }
  cp15_force_cache_coherence((void *) buff,(void*)((UNS_8 *)buff + (AUDIOBUFFSIZE/2)*sizeof(UNS_32) - 1));
  //_tci_loop();
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
void dma_callback(INT_32 _status)
{
  INT_32 done = 0;
#if 0  
//DEBUG  
  if(_half_buff&0x1)
    GPIO->p3_outp_set |= 1<<28;
#endif
  
  
  
  
#if 1  //_PORT
    if(!(_half_buff&0x1))
    /* Fill first half of buffer */ 
     done = fill_buff(&dma_buff[0]);
    else
    /* 2nd half */
      done = fill_buff(&dma_buff[AUDIOBUFFSIZE/2]);
   
#endif    

#if 0
//DEBUG 
  if(!(_half_buff&0x1))  
    GPIO->p3_outp_clr |= 1<<28;
#endif
  
    _half_buff++;

}




void _i2s_entry(void) {
  
  int rateidx, ret = _ERROR;
  
  
  
//DEBUG  
  GPIO->p2_dir_set |= 1<<28;
#if 0  
  dma_buff[0]=0x55555555;
  dma_buff[1]=0x00010001;
  dma_buff[2]=0x55550001;
  dma_buff[3]=0x00015555;
  dma_buff[4]=0x00010001;
  dma_buff[5]=0x00010000;
#endif  
  _half_buff=0;

  
  
  //disable_irq();
    /* Enable I2S0 clock */
  clkpwr_clk_en_dis(CLKPWR_I2S0_CLK,1);
  
  i2s = i2s_open(I2S0, 0);
  i2s_ioctl(i2s, I2S_DO_RESET, 1);
  i2s_ioctl(i2s, I2S_DO_RESET, 0);
  _par0.i2s_word_width = I2S_WW32;
  _par0.i2s_tx_slave = 1;
  i2s_ioctl(i2s, I2S_SETUP_PARAMS, *((INT_32 *)&_par0));
  i2s_ioctl(i2s, I2S_DO_MUTE, 0);
  
  i2s_ioctl(i2s, I2Sx_DMA0_TX_EN, 0);
  i2s_ioctl(i2s, I2Sx_DMA0_RX_EN, 0); 
 
  
#if 1 
  dma_init();
  //dmaregs = dma_get_base();
  _dma_par0.dmach = dma_alloc_channel(0, dma_callback); 
  _dma_par0.dir = DMAC_CHAN_FLOW_D_M2P;
  _dma_par0.mem = (INT_32)(&dma_buff[0]);
  _dma_par0.sz = AUDIOBUFFSIZE/2;
  i2s_dma_init_dev(i2s, (I2S_DMA_PRMS_T *)&_dma_par0);
#endif  
  
  //enable_irq();
  
  //while(1){
    //asm("nop");
    
    
  


  
  

  
  
  

  
#if 0  
    /* Let DMA handle audio */
  i2s_ioctl(i2s, I2Sx_DMA0_TX_EN, 0);
  dma_free_channel(_dma_par0.dmach);
#endif

  return;
}



void __dma_free_channel(void )
{
  dma_free_channel(dmach);
}


void init_timer(void)
{
INT_32 i2sTxdev;



  
#if 1  
  tlv320aic23_probe(22050); //инит aic23
#endif
  //on/off booster
#if 0  //_PORT
  _booster(0x01);
  _booster_mux(0x0);
  _booster_silence(0x0);
#endif  

  
  _i2s_entry();
  _init_render();
  fill_buff(&dma_buff[0]);
  fill_buff(&dma_buff[AUDIOBUFFSIZE/2]);

 

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
      //lpc313x_main_clk_rate(sample_rate);
      //lpc313x_chan_clk_enable(CLK_TX_1, sample_rate,(sample_rate * 16*2));
      //_set_master();
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

#if 0 //PORT
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
    //_b_s_down = _fast_down(); //_PORT
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
