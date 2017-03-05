/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2006
 *
 *    File name   : iram_init.c
 *    Description : Low level init device code
 *
 *    History :
 *    1. Date        : December, 15 2006
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 30926 $
 **************************************************************************/
#include <intrinsics.h>
#include <NXP/iolpc3250.h>

/*************************************************************************
 * Function Name: __low_level_init
 * Parameters: none
 *
 * Return: int
 *
 * Description: This function is used for low level initialization
 *
 *************************************************************************/
int __low_level_init(void)
{
  // init clock
  SYSCLK_CTRL = 0x50<<2;
  // Set PER_CLK and HCLK dividers
  HCLKDIV_CTRL = 0x0000003D;
  // init H PLL
  HCLKPLL_CTRL = 0x0001601E;
  // Wait until PLL lock
  while(!HCLKPLL_CTRL_bit.PLL_LOCK);
  // Connect Pll_clk_out
  PWR_CTRL_bit.RUN_MODE = 1;

  BOOT_MAP_bit.MAP = 1;             // iRAM 0x00000000
  return(1);
}
