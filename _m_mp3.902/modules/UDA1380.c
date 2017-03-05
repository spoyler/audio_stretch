/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2009
 *
 *    File name   : UDA1380.c
 *    Description : UDA1380 codec
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
#include "UDA1380.h"
/** local definitions **/
#define UDA1380_ADDR 0x1A
/** default settings **/

/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/

/** public functions **/
unsigned short UDA1380Read(unsigned char reg_addr)
{

union
{
  unsigned char data[2];
  unsigned short val;
} reg ;

t_i2c_mtfr_cfg transfer;

  transfer.slv_addr = UDA1380_ADDR;
  transfer.mode = mode_txrx;
  transfer.txsize = 1;
  transfer.txdata = &reg_addr;
  transfer.rxsize = 2;
  transfer.rxdata = reg.data;

  if(done !=  I2C1_Master_Transfer(&transfer)) I2C1_Master_Init();

  return reg.val;
}

void UDA1380Write(unsigned char reg_addr, unsigned short val)
{
unsigned char data[3];
  data[0]  = reg_addr;
  data[1]  = val>>8;
  data[2]  = val&0xff;

  t_i2c_mtfr_cfg transfer;

  transfer.slv_addr = UDA1380_ADDR;
  transfer.mode = mode_tx;
  transfer.txsize = 3;
  transfer.txdata = data;
  if(done !=  I2C1_Master_Transfer(&transfer)) I2C1_Master_Init();
}

void UDA1380Init(void)
{
  I2C1_Master_Init(); /*Init I2C interface*/
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_PWR_CTRL, 0x241F); //0xA500
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_I2S_BUS, 0x0000); //
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_EVAL_MODES, 0x0F0A); //0x0702
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_MASTER_MUT, 0x0000);
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_DMTR_VOL, 0xF3F3);
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_ADC, 0x0002);
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_AGC, 0x0000);
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_PGA, 0x0000);

  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_ANL_MIX, 0x3F3F);
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_HPHONE_AMP, 0xefef);
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_MASTER_VOL, 0x0000);
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_MIX_VOL, 0xFF00);
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_MODE_SEL, 0x0000);
  for(volatile int i = 0; 5000 > i; i++);
  UDA1380Write(UDA1380_MIXER, 0x0000);
}
/** private functions **/
