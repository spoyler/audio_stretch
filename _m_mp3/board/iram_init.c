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
#include "arm926ej_cp15_drv.h"
#include "ttbl.h"
#include "lpc_types.h"
#include "lpc_irq_fiq.h"
#include "lpc32xx_intc_driver.h"
#include "_pll.h"
#include "lpc32xx_slcnand_driver.h"
#include "lpc32xx_gpio_driver.h"


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
	// Init MMU
  CP15_Mmu(FALSE);            // Dis able MMU
  CP15_Cache(FALSE);          //cach dis
  // Privileged permissions  User permissions AP
  // Read-only               Read-only        0 
  CP15_SysProt(FALSE);
  CP15_RomProt(TRUE);
  CP15_InitMmuTtb(TtSB,TtTB); // Build L1 and L2 Translation  tables
  CP15_SetTtb(L1Table);       // Set base address of the L1 Translation table
  CP15_SetDomain( (DomainManager << 2*1) | (DomainClient << 0)); // Set domains
  CP15_Mmu(TRUE);             // Enable MMU
  CP15_Cache(TRUE);           // Enable ICache,DCache

  /* Disable interrupts in ARM core */
  disable_irq();
  
  _init_pll();

  /* Initialize interrupt system */
  int_initialize(0xFFFFFFFF);
  
  
//инит в boot0 
#if 1  
  
  sdr_sdram_setup(AHB_CLK);
#endif  

  
  
#if 0  
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
#endif
  return(1);
}
