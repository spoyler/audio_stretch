/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2009
 *
 *    File name   : drv_i2c.c
 *    Description : Lpc3130 I2C Driver
 *
 *    History :
 *    1. Date        : 10.01.2010 
 *       Author      : Stoyan Choynev
 *       Description : Initila Revision
 *
 *    $Revision: 30870 $
 *************************************************************************/
/** include files **/
#include <NXP\iolpc3130.h>
#include "arm_comm.h"
#include "drv_i2c.h"
/** local definitions **/

/** default settings **/
#define I2C_MAIN_CLOCK  6MHZ
#define I2C_MAXSPEED    400000 /**/
#define I2C_SPEED       100000
/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/

/** public functions **/
void I2C1_Master_Init(void)
{
  /*Soft Reset of the I2C1 module*/
  I2C1_CTL = (1<<8);
  while((I2C1_CTL & (1<<8)));
  /*Set Frequency to 100000KHz*/
  I2C1_CLK_HI = I2C_MAIN_CLOCK/(I2C_SPEED);
  I2C1_CLK_LO = (I2C_MAIN_CLOCK+(I2C_SPEED/2))/(I2C_SPEED);
  
  I2C1_ADR = 0x7F;
  
}
__no_init volatile int I2C1_STAT @ 0x1300A404;

t_i2c_res I2C1_Master_Transfer(t_i2c_mtfr_cfg * trf_cfg)
{
volatile unsigned int dummy;

  if(I2C1_STAT&(1<<5)) return busy;
  I2C1_STAT = 0x3;

  do
  {
    I2C1_CTL = 0x6F;
    /*Start + Send slave adders + R/W*/
    I2C1_TX = (1<<8) | ((trf_cfg->slv_addr)<<1) | ((trf_cfg->mode)&0x1);
    /*Mode check*/
    if((trf_cfg->mode)&0x1)
    {/*Master receive*/      
      unsigned int size = 0;
      unsigned int txsize = 0;
      while(size != trf_cfg->rxsize)
      {
         if(!(I2C1_STS&(1<<9)))
         {
          *trf_cfg->rxdata = I2C1_RX;
          trf_cfg->rxdata++;
          size++;
         }

        if((txsize < trf_cfg->rxsize) && !(I2C1_STS&(1<<10)))
        {
          I2C1_TX = ((++txsize == trf_cfg->rxsize)?(1<<9 | 0xAA):(0xAA));
        }               
      }
    }
    else
    {/*Master transmit*/
      unsigned int size = 0;
      while(size != trf_cfg->txsize)
      {
        if(!(I2C1_STS&(1<<10)))
        {/**/
          I2C1_TX = *trf_cfg->txdata | (((++size == trf_cfg->txsize) && (0==trf_cfg->mode))?(1<<9):(0));
          trf_cfg->txdata++;
        }
      }
    }
   /*wait end of the transmision*/
   while(!((dummy = I2C1_STS)&(1<<11)))
   {
    /*check for error*/
    if(I2C1_STS&(1<<2)) return nack;
    if(I2C1_STS&(1<<1)) return af;
   }
  }
  while(0!=(trf_cfg->mode>>=1));
  

  return done;
}
/** private functions **/
