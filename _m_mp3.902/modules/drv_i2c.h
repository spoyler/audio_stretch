/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2009
 *
 *    File name   : drv_i2c.h
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

#ifndef __I2C_H
#define __I2C_H
/** definitions **/
#define I2C0  0x1300a000
#define I2C1  0x1300a400

typedef enum
{
  mode_tx = 0,
  mode_rx,
  mode_txrx
} t_i2c_mode;

typedef enum
{
  done =0,
  nack,
  af,
  busy
} t_i2c_res;

typedef struct
{
  t_i2c_mode mode;
  unsigned char slv_addr;
  unsigned char * txdata;
  unsigned int txsize;
  unsigned char * rxdata;
  unsigned int rxsize;
} t_i2c_mtfr_cfg;
/** public data **/

/** public functions **/
void I2C1_Master_Init(void);
t_i2c_res I2C1_Master_Transfer(t_i2c_mtfr_cfg * trf_cfg);

#endif /*__I2C_H*/