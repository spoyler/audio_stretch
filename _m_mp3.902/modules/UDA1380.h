/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2009
 *
 *    File name   : UDA1380.h
 *    Description : UDA1380 Header file
 *
 *    History :
 *    1. Date        : 10.01.2010 
 *       Author      : Stoyan Choynev
 *       Description : Initila Revision
 *
 *    $Revision: 30870 $
 *************************************************************************/
/** include files **/

#ifndef __UDA1380_H
#define __UDA1380_H
/** definitions **/
#define UDA1380_EVAL_MODES  0x00
#define UDA1380_I2S_BUS     0x01
#define UDA1380_PWR_CTRL    0x02
#define UDA1380_ANL_MIX     0x03
#define UDA1380_HPHONE_AMP  0x04
#define UDA1380_MASTER_VOL  0x10
#define UDA1380_MIX_VOL     0x11
#define UDA1380_MODE_SEL    0x12
#define UDA1380_MASTER_MUT  0x13
#define UDA1380_MIXER       0x14
#define UDA1380_DMTR_VOL    0x20
#define UDA1380_PGA         0x21
#define UDA1380_ADC         0x22
#define UDA1380_AGC         0x23
#define UDA1380_RESET       0x7F
#define UDA1380_IFILTER     0x18
#define UDA1380_DMTR        0x28
/** public data **/

/** public functions **/
unsigned short UDA1380Read(unsigned char reg_addr);
void UDA1380Write(unsigned char reg_addr, unsigned short val);
void UDA1380Init(void);


#endif /*__UDA1380_H*/