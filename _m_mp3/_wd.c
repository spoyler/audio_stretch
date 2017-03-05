//��������: ���������� WD 



#include "includes.h"
//#include "lpc_irq_fiq.h"  //DEBUG
#include "lpc_types.h"
#include "lpc_irq_fiq.h"
#include "lpc32xx_intc_driver.h"

//reset WD
//�������� ����� �������� _reset_WD ������ ���� ����� min ��������� DW
void _reset_WD()
{
#if 0//FIX_DEBUG
    __disable_interrupt(); 
    WDMOD &= ~(0x01<<2);
    WDFEED = 0xAA;
    WDFEED = 0x55;
    __enable_interrupt(); 
#endif
  
}


//���� WD
void _init_WD()
{
#if 0//FIX_DEBUG  
  WDMOD = 0x03; //������ RESET
  //WDTC = 0x0Affffff; //� PCLK = 60Mhz WD �������� �� 8���
  WDTC = 0x03ffffff; //� PCLK = 60Mhz WD �������� �� 8���
  
  
  WDFEED = 0xAA;
  WDFEED = 0x55;
#endif  
}


//������� ���� WD
void _fast_init_WD()
{
#if 0  
  WDMOD = 0x03; //������ RESET
  //WDTC = 0x0Affffff; //� PCLK = 60Mhz WD �������� �� 10���
  WDTC = 0x000001ff; 
#endif  
}

void _en_irq(void)
{
  enable_irq();
}

void _dis_irq(void)
{
  disable_irq(); //DEBUG
}


/*
  ������ ����� _boot2 dec2hex(hex2dec('11157FFF')-hex2dec('11157704'))= 0x8FB
*/
#define _BOOT2_SIZE  0x8FB

//-----------------------------------
#define _BOOT2_FLASH_OFFSET 0x2e704 //����� �� _boot2.bin
//��������� adr ������������ ���������� _boot2, ���� �� entry point �� jtag
static unsigned long _boot2_start = 0x11157704;
#if 1
void _run(void)
{
  //������� ����� �� ��� _boot2_start
  ;//asm("  b  0x11157704 ");
}
#endif
//-----------------------------------------




