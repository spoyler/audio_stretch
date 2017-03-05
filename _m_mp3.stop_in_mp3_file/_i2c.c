/***********************************************************************
 * $Id:: i2c_example.c 1332 2008-11-19 21:20:07Z tangdz                $
 *
 * Project: NXP PHY3250 I2C example
 *
z *
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
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_i2c_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc_irq_fiq.h"
#include "max98089_dr.h"

#define _SP  0x01
#define _HP  (0x01<<1)

extern volatile unsigned char _modes;


static uint32_t do_div(uint64_t *n, uint32_t base) {
uint32_t remainder = *n % base;

  *n = *n / base;

  return remainder;
}
           

#define _MCLK 13000000

/***********************************************************************
 *
 * Function: delay
 *
 * Purpose: generate a delay
 *
 * Processing:
 *     A local software counter counts up to the specified count.
 *
 * Parameters:
 *    cnt : number to be counted
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
static void delay(UNS_32 cnt)
{
    UNS_32 i = cnt;
    while (i != 0) i--;
    return;
}

/* I2C device handles */
static INT_32 i2cdev1;

static UNS_8 iob[3] = {0,0,0};
void __set_sample_rate(unsigned short _sample_rate){
  
  //CLK=12.288
  switch(_sample_rate){
  case 8000:
    iob[0] = 0x10;
    iob[1] = 0x10;//1000 
    iob[2] = 0x00;//    
    break;
  case 11025:
    iob[0] = 0x20;
    iob[1] = 0x16;//1000 
    iob[2] = 0x0d;//    
    break;
  case 16000:
    iob[0] = 0x30;
    iob[1] = 0x20;//1000 
    iob[2] = 0x00;//    
    break;
  case 22050:
    iob[0] = 0x40;
    iob[1] = 0x2c;//1000 
    iob[2] = 0x1a;//    
    break;
  case 24000:
    iob[0] = 0x50;
    iob[1] = 0x30;//1000 
    iob[2] = 0x00;//    
    break;
  case 32000:
    iob[0] = 0x60;
    iob[1] = 0x40;//1000 
    iob[2] = 0x00;//    
    break;
  case 44100:
    iob[0] = 0x70;
    iob[1] = 0x58;//1000 
    iob[2] = 0x33;//    
    break;
  case 48000:
    iob[0] = 0x80;
    iob[1] = 0x60;//1000 
    iob[2] = 0x00;//    
    break;
   
  default:
    iob[0] = 0x40;
    iob[1] = 0x2c;//1000 
    iob[2] = 0x1a;//    
    break;
  };
  
  write_max98((UNS_32)i2cdev1, 0x11, 3, iob);
}

void _max98089_volume(unsigned char _i0){
  unsigned char _rbuff0;
  
  
  if(_modes&_SP){
    write_max98((UNS_32)i2cdev1,0x3D,1,&_i0);
    read_max98((UNS_32)i2cdev1,0x3D,1,&_rbuff0); 
    write_max98((UNS_32)i2cdev1,0x3E,1,&_i0);
    read_max98((UNS_32)i2cdev1,0x3E,1,&_rbuff0);
  }
  if(_modes&_HP){
    write_max98((UNS_32)i2cdev1,0x39,1,&_i0);
    read_max98((UNS_32)i2cdev1,0x39,1,&_rbuff0); 
    write_max98((UNS_32)i2cdev1,0x3A,1,&_i0);
    read_max98((UNS_32)i2cdev1,0x3A,1,&_rbuff0);
  }
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
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/


void _i2c_entry(unsigned short _sample_rate)
{
  I2C_SETUP_T i2c_setup[2];
  I2C_MTX_SETUP_T   i2c_mtx_setup;
  

#if 1 //настройка adr для Si4704 
  GPIO->p2_dir_set |= (0x1<<26); //GPIO_0 to out
  //GPIO->p3_outp_set |= (0x1<<26); //GPIO_0 set, adr 1100011b
  GPIO->p3_outp_clr |= (0x1<<26); //GPIO_0 unset, adr 0010001b
#endif

  /* Enable I2C1/2 clock */
  clkpwr_clk_en_dis(CLKPWR_I2C1_CLK,1);
  

  /* install default I2C1 & I2C2 interrupt handlers */
  int_install_ext_irq_handler(IRQ_I2C_1, 
                              (PFV) i2c1_user_interrupt, ACTIVE_LOW, 1);
  /*int_install_ext_irq_handler(IRQ_I2C_2, 
                              (PFV) i2c2_user_interrupt, ACTIVE_LOW, 1);*/

  enable_irq();
  
  /* open I2C1 */
   i2cdev1 = i2c_open(I2C1, 0);

  /* formally assign a 7-bit slave address 0x50 to I2C1    */
  /* I2C1 clock is 100 kHz, 50% duty cycle, high pin drive */
  i2c_setup[0].addr_mode  = ADDR7BIT;
  //i2c_setup[0].sl_addr    = 0x10;
  i2c_setup[0].rate_option= I2C_RATE_RELATIVE;
  i2c_setup[0].rate       = 100000;
  i2c_setup[0].low_phase  = 50;
  i2c_setup[0].high_phase = 50;
  i2c_setup[0].pins_drive = I2C_PINS_HIGH_DRIVE;
  i2c_ioctl((UNS_32) i2cdev1, I2C_SETUP, (UNS_32) &i2c_setup[0]);
  
 

_MAX98_REVISION_ID _rev_id; 
  Read_DSP_REVISION_ID((UNS_32)i2cdev1,&_rev_id);
  

_MAX98_POWER_MANAGEMENT _power,_rpower;    
   Read_POWER_MANAGEMENT((UNS_32)i2cdev1,&_power); 
  _power.regSystemShutdown.bit_reg.PWRSV = 0;
  _power.regSystemShutdown.bit_reg.SHDN = 0;
  _power.regTopLevelBiasControl.char_reg = 0xF0;
  _power.regDACLowPowerMode2.char_reg = 0x0F;
  if(_modes&_SP)
    _power.regOutputEnable.char_reg = M98088_DAREN | M98088_DALEN | M98088_SPLEN | M98088_SPREN;
  if(_modes&_HP)
    _power.regOutputEnable.char_reg = M98088_DAREN | M98088_DALEN | M98088_HPLEN | M98088_HPREN;  
  Write_POWER_MANAGEMENT((UNS_32)i2cdev1,&_power);
  Read_POWER_MANAGEMENT((UNS_32)i2cdev1,&_rpower); //test
 
  
  
#if 0
//не работает!!!!!!  
_MAX98_LEVEL_CONTROL _level,_rlevel;
  Read_LEVEL_CONTROL((UNS_32)i2cdev1,&_level);
  _level.regDAI1PlaybackLevel.bit_reg.DVM = 0; ///???? may be 0
  if(_modes&_SP){
    _level.RightSpeakerAmplifierVolumeControl.bit_reg.SPM = 0;
    _level.RightSpeakerAmplifierVolumeControl.bit_reg.SPVOL = 0x15;
    _level.LeftSpeakerAmplifierVolumeControl.bit_reg.SPM = 0;
    _level.LeftSpeakerAmplifierVolumeControl.bit_reg.SPVOL = 0x15;
  }
  if(_modes&_HP){
    _level.RightHeadphoneAmplifierVolumeControl.bit_reg.HPM = 0;
    _level.RightHeadphoneAmplifierVolumeControl.bit_reg.HPVOL = 0x1A;
    _level.LeftHeadphoneAmplifierVolumeControl.bit_reg.HPM = 0;
    _level.LeftHeadphoneAmplifierVolumeControl.bit_reg.HPVOL = 0x1A;
  }  
  Write_LEVEL_CONTROL((UNS_32)i2cdev1,&_level);
  Read_LEVEL_CONTROL((UNS_32)i2cdev1,&_rlevel);  
#else
//работает !!!!!  установка громкости
unsigned char _buff0[2],_rbuff0[2];
  if(_modes&_SP){
    _buff0[0] = 0x10;
    write_max98((UNS_32)i2cdev1,0x3D,1,&_buff0);
    read_max98((UNS_32)i2cdev1,0x3D,1,_rbuff0); 
    write_max98((UNS_32)i2cdev1,0x3E,1,&_buff0);
    read_max98((UNS_32)i2cdev1,0x3E,1,_rbuff0);
  }
  if(_modes&_HP){
    _buff0[0] = 0x0A;
    write_max98((UNS_32)i2cdev1,0x39,1,&_buff0);
    read_max98((UNS_32)i2cdev1,0x39,1,_rbuff0); 
    write_max98((UNS_32)i2cdev1,0x3A,1,&_buff0);
    read_max98((UNS_32)i2cdev1,0x3A,1,_rbuff0);
  }  
#endif  
 
    
    
    
_MAX98_MIXERS _mixer,_rmixer;
  Read_MIXERS((UNS_32)i2cdev1,&_mixer);
  _mixer.regDACMixer.char_reg = M98088_DAI1L_TO_DACL | M98088_DAI1R_TO_DACR;
  if(_modes&_SP){  
    _mixer.regRightSpeakerAmplifierMixer.char_reg = 0x80;
    _mixer.regLeftSpeakerAmplifierMixer.char_reg = 0x01;
  }
  if(_modes&_HP){
    _mixer.regRighttHeadphoneAmplifierMixer.char_reg = 0x80;
    _mixer.regLeftHeadphoneAmplifierMixer.char_reg = 0x01;    
  }
  Write_MIXERS((UNS_32)i2cdev1,&_mixer);
  Read_MIXERS((UNS_32)i2cdev1,&_rmixer);

  
 
  
//-------------------------  
_MAX98_STATUS _status; 
  Read_STATUS((UNS_32)i2cdev1,&_status);
  _status.regInterruptEnable.char_reg = 0;
  Write_STATUS((UNS_32)i2cdev1,&_status);
  
  


_MAX98_DAI1_CONFIGURATION _dai1_conf,_rdai1_conf;
  Read_DAI1_CONFIGURATION((UNS_32)i2cdev1,&_dai1_conf);
  _dai1_conf.IOConfiguration.char_reg =  M98088_S1NORMAL | M98088_SDATA ;
//настройки режимов I2S  
  _dai1_conf.Format.bit_reg.MAS = 1;
  _dai1_conf.Format.bit_reg.DLY = 1;
  _dai1_conf.Format.bit_reg.BCI = 0;
  _dai1_conf.Format.bit_reg.WCI = 0;
  _dai1_conf.Format.bit_reg.WS = 0;
  _dai1_conf.Format.bit_reg.TDM = 0;
  _dai1_conf.regFilters.bit_reg.DHF = 0;
  _dai1_conf.regClock.bit_reg.BSEL = 0x1;
  Write_DAI1_CONFIGURATION((UNS_32)i2cdev1,&_dai1_conf);
  Read_DAI1_CONFIGURATION((UNS_32)i2cdev1,&_rdai1_conf);


_MAX98_MASTER_CLOCK_CONTROL _mCLK;
  _mCLK.regMasterClock.char_reg = 0x10;
  Write_MASTER_CLOCK_CONTROL((UNS_32)i2cdev1,&_mCLK);  
  
/*
5 } rate_table[] = {
936        {8000,  0x10},
937        {11025, 0x20},
938        {16000, 0x30},
939        {22050, 0x40},
940        {24000, 0x50},
941        {32000, 0x60},
942        {44100, 0x70},
943        {48000, 0x80},
944        {88200, 0x90},
945        {96000, 0xA0},  
*/
_MAX98_DAI1_CLOCK_CONTROL _frame_rate,_rframe_rate;
unsigned long long ni;
unsigned int rate;
#if 0
  Read_DAI1_CLOCK_CONTROL((UNS_32)i2cdev1,&_frame_rate);
  rate = 22050;
  _frame_rate.regClockMode.bit_reg.SR = 0x4; //22050Hz
  _frame_rate.regClockMode.bit_reg.FREQ = 0xa; //LRCLK=8kHz
#if 0  
  ni = 65536ULL * (  rate  < 50000 ? 96ULL : 48ULL) * (unsigned long long)rate; 
  do_div(&ni, _MCLK);
  _frame_rate.AnyClockControl.short_reg = (((ni >> 8) & 0x7F)<<8)+(ni&0xFF);
#endif  
  Write_DAI1_CLOCK_CONTROL((UNS_32)i2cdev1,&_frame_rate);
  Read_DAI1_CLOCK_CONTROL((UNS_32)i2cdev1,&_rframe_rate);
#endif  
//-------------------  
  



  __set_sample_rate(_sample_rate);

  

  
   Read_POWER_MANAGEMENT((UNS_32)i2cdev1,&_power); 
  _power.regSystemShutdown.bit_reg.SHDN = 1;
  Write_POWER_MANAGEMENT((UNS_32)i2cdev1,&_power);

  
  
  
  
  /* close I2C1 */    
  //i2c_close(i2cdev1);
}
