#ifndef _PLL_CLK_H
  #define _PLL_CLK_H

// System OSC 13MHz
#define OSC           (13000000UL)
#define HCLK_DIV      2
#define PCLK_DIV      16
// ARM_CLK 208MHz
#define ARM_CLK_MHZ      (OSC*16)
// HCLK 104MHz
#define AHB_CLK       (ARM_CLK_MHZ/HCLK_DIV)
// PER_CLK 13MHz
#define PER_CLK       (ARM_CLK_MHZ/PCLK_DIV)
// RTC_CLK
#define RTC_CLK       (32768UL)
#define TEST_CLK1     0x400040A4


#endif /* VAL3250_BOARD_H */
