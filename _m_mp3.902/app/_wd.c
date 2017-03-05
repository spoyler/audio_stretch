//Описание: реализация WD 



#include "lpc313x_cgu_driver.h"
#include "lpc313x_wdt.h"



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
void _init_WD(void)
{
    /* enable WDT clock */
    cgu_clk_en_dis(CGU_SB_WDOG_PCLK_ID, 1);
    WDT->tcr = 0;
    WDT->mcr = WDT_MCR_RESET_MR0 | WDT_MCR_INT_MR0;  
    //WDT->pr  = 0x00000002;
    //WDT->tc  = 0x00000FF0;
    WDT->mr0 = 0x00000003;
    //WDT->mr1 = 0x00001000;
    WDT->emr = WDT_EMR_CTRL1(0x1);    
    WDT->tcr = WDT_TCR_CNT_EN;
    while(1);
}

void _soft_intr(void)
{
  asm(" SWI 0x0  ");
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




/*
  размер проги _boot2 dec2hex(hex2dec('11157FFF')-hex2dec('11157898'))= 0x767
*/
#define _BOOT2_SIZE  0x767

//-----------------------------------
#define _BOOT2_FLASH_OFFSET 0x0194 //взято из _boot2.bin
//стартовый adr резидентного загрузчика _boot2, взят из entry point по jtag
static unsigned long _boot2_start = 0x11157898;
#if 0
//в cstartup.s
void _run(void)
{
  //переход такой же как _boot2_start
  asm("  LDR r0, =0x11157898 ");
  asm("  bx r0 ");
  //asm("  b  0x11157898 ");
}
#endif
//-----------------------------------------




