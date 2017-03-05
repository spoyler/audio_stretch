/**
Драйвер audio кодека MAX98089
**/

#include "max98089_dr.h"
#include "lpc32xx_i2c_driver.h"

_MAX98_DSP_COEFFICIENTS _band_DSP_COEFFICIENTS1;
_MAX98_DSP_COEFFICIENTS _band_DSP_COEFFICIENTS2;

static unsigned char _buff[50];
int write_max98(INT_32 devid,int address,int n_bytes,char *buffer) {
   char i;
   I2C_MTX_SETUP_T   i2c_mtx_setup;
   
   
    _buff[0] = address;
    memcpy(&_buff[1],buffer,n_bytes);
    i2c_mtx_setup.addr_mode = ADDR7BIT;
    i2c_mtx_setup.sl_addr = 0x10;
    i2c_mtx_setup.tx_data = (unsigned char *)&_buff[0];
    i2c_mtx_setup.tx_length = (1+n_bytes);
    i2c_mtx_setup.retransmissions_max = 5;
    i2c_ioctl(devid, I2C_MASTER_TX, (INT_32) &i2c_mtx_setup);
    while (( i2c_mtx_setup.status & I2C_SETUP_STATUS_DONE) == 0);   
   
    
   return 0;
}

int read_max98(INT_32 devid,int address,int n_bytes,char *buffer) {
  
  I2C_MTXRX_SETUP_T i2c_mtxrx_setup; 
  
    i2c_mtxrx_setup.addr_mode = ADDR7BIT;
    i2c_mtxrx_setup.sl_addr = 0x10;
    i2c_mtxrx_setup.tx_data = (unsigned char *)&address;
    i2c_mtxrx_setup.tx_length = 1;
    i2c_mtxrx_setup.rx_data = buffer;
    i2c_mtxrx_setup.rx_length = n_bytes;
    i2c_mtxrx_setup.retransmissions_max = 5;
    i2c_ioctl(devid, I2C_MASTER_TXRX, (INT_32) &i2c_mtxrx_setup);
    while (( i2c_mtxrx_setup.status & I2C_SETUP_STATUS_DONE) == 0);
   
   
   return 0;
}



int Write_STATUS(INT_32 devid,_MAX98_STATUS* data)
{
   write_max98(devid,0x03,1,&data->regBatteryVoltage.char_reg);
   write_max98(devid,0x0f,1,&data->regInterruptEnable.char_reg);
   return 0;
}
int Read_STATUS(INT_32 devid,_MAX98_STATUS* data)
{
   read_max98(devid,_ADDRESS_STATUS,4,(char*)data);
   read_max98(devid,0x0f,1,&data->regInterruptEnable.char_reg);
   return 0;
}

int Write_MASTER_CLOCK_CONTROL(INT_32 devid,_MAX98_MASTER_CLOCK_CONTROL* data)
{
   write_max98(devid,_ADDRESS_MASTER_CLOCK_CONTROL,1,(char*)data);
   return 0;
}
int Read_MASTER_CLOCK_CONTROL(INT_32 devid,_MAX98_MASTER_CLOCK_CONTROL* data)
{
   read_max98(devid,_ADDRESS_MASTER_CLOCK_CONTROL,1,(char*)data);
   return 0;
}

int Write_DAI1_CLOCK_CONTROL(INT_32 devid,_MAX98_DAI1_CLOCK_CONTROL* data)
{
   write_max98(devid,_ADDRESS_DAI1_CLOCK_CONTROL,3,(char*)data);
   return 0;
}
int Read_DAI1_CLOCK_CONTROL(INT_32 devid,_MAX98_DAI1_CLOCK_CONTROL* data)
{
   read_max98(devid,_ADDRESS_DAI1_CLOCK_CONTROL,3,(char*)data);
   return 0;
}

int Write_DAI1_CONFIGURATION(INT_32 devid,_MAX98_DAI1_CONFIGURATION* data)
{
   write_max98(devid,_ADDRESS_DAI1_CONFIGURATION,5,(char*)data);
   return 0;
}
int Read_DAI1_CONFIGURATION(INT_32 devid,_MAX98_DAI1_CONFIGURATION* data)
{
   read_max98(devid,_ADDRESS_DAI1_CONFIGURATION,5,(char*)data);
   return 0;
}
int Write_DAI2_CLOCK_CONTROL(INT_32 devid,_MAX98_DAI2_CLOCK_CONTROL* data)
{
   write_max98(devid,_ADDRESS_DAI2_CLOCK_CONTROL,3,(char*)data);
   return 0;
}
int Read_DAI2_CLOCK_CONTROL(INT_32 devid,_MAX98_DAI2_CLOCK_CONTROL* data)
{
   read_max98(devid,_ADDRESS_DAI2_CLOCK_CONTROL,3,(char*)data);
   return 0;
}

int Write_DAI2_CONFIGURATION(INT_32 devid,_MAX98_DAI2_CONFIGURATION* data)
{
   write_max98(devid,_ADDRESS_DAI2_CONFIGURATION,5,(char*)data);
   return 0;
}
int Read_DAI2_CONFIGURATION(INT_32 devid,_MAX98_DAI2_CONFIGURATION* data)
{
   read_max98(devid,_ADDRESS_DAI2_CONFIGURATION,5,(char*)data);
   return 0;
}

int Write_SRC(INT_32 devid,_MAX98_SRC* data)
{
   write_max98(devid,_ADDRESS_SRC,1,(char*)data);
   return 0;
}
int Read_SRC(INT_32 devid,_MAX98_SRC* data)
{
   read_max98(devid,_ADDRESS_SRC,1,(char*)data);
   return 0;
}

int Write_MIXERS(INT_32 devid,_MAX98_MIXERS* data)
{
   write_max98(devid,_ADDRESS_MIXERS,12,(char*)data);
   return 0;
}
int Read_MIXERS(INT_32 devid,_MAX98_MIXERS* data)
{
   read_max98(devid,_ADDRESS_MIXERS,12,(char*)data);
   return 0;
}

int Write_LEVEL_CONTROL(INT_32 devid,_MAX98_LEVEL_CONTROL* data)
{
   write_max98(devid,_ADDRESS_LEVEL_CONTROL,17,(char*)data);
   return 0;
}
int Read_LEVEL_CONTROL(INT_32 devid,_MAX98_LEVEL_CONTROL* data)
{
   read_max98(devid,_ADDRESS_LEVEL_CONTROL,17,(char*)data);
   return 0;
}

int Write_MICROPHONE_AGC(INT_32 devid,_MAX98_MICROPHONE_AGC* data)
{
   write_max98(devid,_ADDRESS_MICROPHONE_AGC,2,(char*)data);
   return 0;
}
int Read_MICROPHONE_AGC(INT_32 devid,_MAX98_MICROPHONE_AGC* data)
{
   read_max98(devid,_ADDRESS_MICROPHONE_AGC,2,(char*)data);
   return 0;
}

int Write_SPEAKER_SIGNAL_PROCESSING(INT_32 devid,_MAX98_SPEAKER_SIGNAL_PROCESSING* data)
{
   write_max98(devid,_ADDRESS_SPEAKER_SIGNAL_PROCESSING,6,(char*)data);
   return 0;
}
int Read_SPEAKER_SIGNAL_PROCESSING(INT_32 devid,_MAX98_SPEAKER_SIGNAL_PROCESSING* data)
{
   read_max98(devid,_ADDRESS_SPEAKER_SIGNAL_PROCESSING,6,(char*)data);
   return 0;
}

int Write_CONFIGURATION(INT_32 devid,_MAX98_CONFIGURATION* data)
{
   write_max98(devid,_ADDRESS_CONFIGURATION,5,(char*)data);
   return 0;
}
int Read_CONFIGURATION(INT_32 devid,_MAX98_CONFIGURATION* data)
{
   read_max98(devid,_ADDRESS_CONFIGURATION,5,(char*)data);
   return 0;
}

int Write_POWER_MANAGEMENT(INT_32 devid,_MAX98_POWER_MANAGEMENT* data)
{
   write_max98(devid,_ADDRESS_POWER_MANAGEMENT,6,(char*)data);
   return 0;
}
int Read_POWER_MANAGEMENT(INT_32 devid,_MAX98_POWER_MANAGEMENT* data)
{
   read_max98(devid,_ADDRESS_POWER_MANAGEMENT,6,(char*)data);
   return 0;
}

int Write_DSP_COEFFICIENTS1(INT_32 devid,_MAX98_DSP_COEFFICIENTS* data)
{
   write_max98(devid,_ADDRESS_DSP_COEFFICIENTS1,50,(char*)data);
   write_max98(devid,0xb6,10,(char*)&data->bandExcursionLimiterBiquad);
   return 0;
}
int Read_DSP_COEFFICIENTS1(INT_32 devid,_MAX98_DSP_COEFFICIENTS* data)
{
   read_max98(devid,_ADDRESS_DSP_COEFFICIENTS1,50,(char*)data);
   read_max98(devid,0xb6,10,(char*)&data->bandExcursionLimiterBiquad);
   return 0;
}

int Write_DSP_COEFFICIENTS2(INT_32 devid,_MAX98_DSP_COEFFICIENTS* data)
{
   write_max98(devid,_ADDRESS_DSP_COEFFICIENTS2,50,(char*)data);
   write_max98(devid,0xc0,10,(char*)&data->bandExcursionLimiterBiquad);
   return 0;
}
int Read_DSP_COEFFICIENTS2(INT_32 devid,_MAX98_DSP_COEFFICIENTS* data)
{
   read_max98(devid,_ADDRESS_DSP_COEFFICIENTS2,50,(char*)data);
   read_max98(devid,0xc0,10,(char*)&data->bandExcursionLimiterBiquad);
   return 0;
}

int Read_DSP_REVISION_ID(INT_32 devid,_MAX98_REVISION_ID* data)
{
   read_max98(devid,_ADDRESS_REVISION_ID,1,(char*)data);
   return 0;
}

