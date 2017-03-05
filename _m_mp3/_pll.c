/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2006
 *
 *    File name   : main.c
 *    Description : Define main module
 *
 *    History :
 *    1. Date        : 12, September 2006
 *       Author      : Stanimir Bonev
 *       Description :
 *
 *  This example project shows how to use the IAR Embedded Workbench for ARM
 * to develop code for the Phytec LPC3250 evaluation board. It shows
 * basic use of CP15, I/O, timer, uart and the interrupt controllers.
 * It starts by blinking LED1 and LED2. The button BTN1 change
 * blinking speed while the button BTN2 change the pattern.  The button
 * action will be displayed to the U5 BD9 connector at 15200-baud, 8-bit,
 * 1-stop and no parity/flow control.
 *  The jumpers settings are regarding Phytec LPC3250 user manual (default)
 *
 *    $Revision: 30926 $
 **************************************************************************/


#include <intrinsics.h>
#include <nxp/iolpc3250.h>
#include <stdio.h>
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_gpio_driver.h"

#include "_pll.h"




void _init_pll(void){
UNS_32 newarmclk, newhclk, newpclk, armclk, hclk, pclk;  
CLKPWR_HCLK_PLL_SETUP_T pllcfg;
UNS_32 freqr;
unsigned char *_test_clk;

#if 0 //PCLK на out 
  GPIO->p2_mux_set |= 0x1;
  _test_clk = (unsigned char *)TEST_CLK1;
  *_test_clk |= 0x1<<4;
#endif  

 
  /* Get the current ARM clock, HCLK, and PCLK speeds */
  armclk = clkpwr_get_base_clock_rate(CLKPWR_ARM_CLK);
  hclk = clkpwr_get_base_clock_rate(CLKPWR_HCLK);
  pclk = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK);

  /* Compute the required ARM clock rate in MHz */
  newarmclk = ARM_CLK_MHZ;

  /* Set PCLK to '1' in direct-run mode */
  clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_NORM, 1, 2);
  clkpwr_set_mode(CLKPWR_MD_DIRECTRUN);

  
  /* Find and set new PLL frequency with a .1% tolerance */
  clkpwr_pll_dis_en(CLKPWR_HCLK_PLL, 0);
  clkpwr_find_pll_cfg(OSC, newarmclk,
                      10, &pllcfg);
  freqr = clkpwr_hclkpll_setup(&pllcfg);
  if (freqr != 0)
  {
    /* Wait for PLL to lock before switching back into RUN mode */
    while (clkpwr_is_pll_locked(CLKPWR_HCLK_PLL) == 0);

    /* Switch out of direct-run mode and set new dividers */
    clkpwr_set_mode(CLKPWR_MD_RUN);
    clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_NORM,
                         PCLK_DIV, HCLK_DIV);
  }
 
#if 0  
  /* Get the current ARM clock, HCLK, and PCLK speeds */
  armclk = clkpwr_get_base_clock_rate(CLKPWR_ARM_CLK);
  hclk = clkpwr_get_base_clock_rate(CLKPWR_HCLK);
  pclk = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK); 
#endif 
  
    
}

