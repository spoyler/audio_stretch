//Описание: реализация WD 



#include "includes.h"
//#include "lpc_irq_fiq.h"  //DEBUG
#include "lpc_types.h"
#include "lpc_irq_fiq.h"
#include "lpc32xx_intc_driver.h"

//reset WD
//интервал между вызовами _reset_WD должен быть более min интервала DW
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


//инит WD
void _init_WD()
{
#if 0//FIX_DEBUG  
  WDMOD = 0x03; //только RESET
  //WDTC = 0x0Affffff; //с PCLK = 60Mhz WD настроен на 8сек
  WDTC = 0x03ffffff; //с PCLK = 60Mhz WD настроен на 8сек
  
  
  WDFEED = 0xAA;
  WDFEED = 0x55;
#endif  
}


//шустрый инит WD
void _fast_init_WD()
{
#if 0  
  WDMOD = 0x03; //только RESET
  //WDTC = 0x0Affffff; //с PCLK = 60Mhz WD настроен на 10сек
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
  размер проги _boot2 dec2hex(hex2dec('11157FFF')-hex2dec('11157704'))= 0x8FB
*/
#define _BOOT2_SIZE  0x8FB

//-----------------------------------
#define _BOOT2_FLASH_OFFSET 0x2e704 //взято из _boot2.bin
//стартовый adr резидентного загрузчика _boot2, взят из entry point по jtag
static unsigned long _boot2_start = 0x11157704;
#if 1
void _run(void)
{
  //переход такой же как _boot2_start
  ;//asm("  b  0x11157704 ");
}
#endif
//-----------------------------------------




