#include "max98089_dr.h"
int main(int argc, char **argv)
{
	INT_32 devid;

	// конфигурирование i2c
/*	devid = i2c_open(I2C1, 0);
	i2c_ioctl(devid, I2C_SETUP, 0);
	i2c_ioctl(devid, I2C_MASTER_TXRX, 0);*/

	// проверка REV_ID
	char id;
	Read_DSP_REVISION_ID(devid, (_MAX98_REVISION_ID*)&id);
	if (id == 0x40)
	{
		// разрешение прерывания ICLD
		_MAX98_STATUS status;
		Read_STATUS(devid, &status);
		status.regInterruptEnable.bit_reg.ICLD = 1;
		Write_STATUS(devid, &status);
	}
	return 0;
}
