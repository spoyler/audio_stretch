/****************************************************************************
 *
 * Project: Generic test routines.
 *
 * Copyright by Olimex Ltd. All rights reserved.
 *
 * File: extcon.h
 * Description: External connector test configuration for LPC-P2919
 * Developer: Dimitar Dimitrov ( dinuxbg,gmail.com )
 *
 * Last change: $Date: 2009/05/15 $
 * Revision: $Revision: #1 $
 * Id: $Id: //depot/IAR/auxiliary_arm2_5_xx/TestCode/NXP/LPC17xx/IAR-LPC-1766-SK/app/extcon-conf.c#1 $
 *
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <NXP\iolpc3131.h>
#include "arm_comm.h"

#include "extcon.h"
#include "uart.h"

#define ARRAY_NUMELEM(A)	(sizeof((A))/sizeof((A)[0]))


#define PIN(CON_PIN, PRT, PRT_PIN, CHIP_PIN)	\
		{ \
			.con_pin = CON_PIN, \
			.port_pin = ((PRT)<<5) | (PRT_PIN), \
			.chip_pin = CHIP_PIN, \
		}

static char * pin_name[] = {
/*EBI_MCI*/
"mGPIO9","mGPIO6","mLCD_DB_7","mLCD_DB_4","mLCD_DB_2","mNAND_RYBN0","mI2STX_CLK0","mI2STX_BCK0",
"EBI_A_1_CLE","EBI_NCAS_BLOUT_0","mLCD_DB_0","EBI_DQM_0_NOE","mLCD_CSB","mLCD_DB_1","mLCD_E_RD","mLCD_RS",
"mLCD_RW_WR","mLCD_DB_3","mLCD_DB_5","mLCD_DB_6","mLCD_DB_8","mLCD_DB_9","mLCD_DB_10","mLCD_DB_11",
"mLCD_DB_12","mLCD_DB_13","mLCD_DB_14","mLCD_DB_15","mGPIO5","mGPIO7","mGPIO8","mGPIO10",
/*EBI_I2STX_0*/
"mNAND_RYBN1","mNAND_RYBN2","mNAND_RYBN3","mUART_CTS_N","mUART_RTS_N","mI2STX_DATA0","mI2STX_WS0","EBI_NRAS_BLOUT_1",
"EBI_A_0_ALE","EBI_NWE","","","","","","",
"","","","","","","","",
"","","","","","","","",
/*CGU*/
"CGU_SYSCLK_O","","","","","","","",
"","","","","","","","",
"","","","","","","","",
"","","","","","","","",
/*I2SRX_0*/
"I2SRX_BCK0","I2SRX_DATA0","I2SRX_WS0","","","","","",
"","","","","","","","",
"","","","","","","","",
"","","","","","","","",
/*I2SRX_1*/
"I2SRX_DATA1","I2SRX_BCK1","I2SRX_WS1","","","","","",
"","","","","","","","",
"","","","","","","","",
"","","","","","","","",
/*I2STX_1*/
"I2STX_DATA1","I2STX_BCK1","I2STX_WS1","I2STX_256FS_O","","","","",
"","","","","","","","",
"","","","","","","","",
"","","","","","","","",
/*EBI*/
"EBI_D_9","EBI_D_10","EBI_D_11","EBI_D_12","EBI_D_13","EBI_D_14","EBI_D_4","EBI_D_0",
"EBI_D_1","EBI_D_2","EBI_D_3","EBI_D_5","EBI_D_6","EBI_D_7","EBI_D_8","EBI_D_15",
"","","","","","","","",
"","","","","","","","",
/*GPIO*/
"GPIO_GPIO1","GPIO_GPIO0","GPIO_GPIO2","GPIO_GPIO3","GPIO_GPIO4","GPIO_GPIO11","GPIO_GPIO12","GPIO_GPIO13",
"GPIO_GPIO14","GPIO_GPIO15","GPIO_GPIO16","GPIO_GPIO17","GPIO_GPIO18","GPIO_GPIO19","GPIO_GPIO20","",
"","","","","","","","",
"","","","","","","","",
/*I2C1*/
"I2C_SDA1","I2C_SCL1","","","","","","",
"","","","","","","","",
"","","","","","","","",
"","","","","","","","",
/*SPI*/
"SPI_MISO","SPI_MOSI","SPI_CS_IN","SPI_SCK","SPI_CS_OUT0","","","",
"","","","","","","","",
"","","","","","","","",
"","","","","","","","",
/*NAND_FLASH*/
"NAND_NCS_3","NAND_NCS_0","NAND_NCS_1","NAND_NCS_2","","","","",
"","","","","","","","",
"","","","","","","","",
"","","","","","","","",
/*PWM*/
"PWM_DATA","","","","","","","",
"","","","","","","","",
"","","","","","","","",
"","","","","","","","",
/*UART*/
"UART_RXD","UART_TXD","","","","","","",
"","","","","","","","",
"","","","","","","","",
"","","","","","","","",
};

static void print_port_pin(char *buf, const struct extcon_pin *pin)
{
	sprintf(buf, "%s", pin_name[pin->port_pin]);
}

static void _puts(char const *buf)
{
        uint32_t Size = strlen(buf);
        uint32_t TranSize = 0;
        do
        {
          Size -= TranSize;
          buf += TranSize;
          TranSize = UartWrite((unsigned char *)buf,Size);
        }
        while(Size != TranSize);
}

static int lpc31xx_gpio_read(const struct extcon_pin *pin)
{
volatile uint32_t * pins =(uint32_t *)&IOCONF_EBI_MCI_PIN + ((pin->port_pin & 0xFFE0)>>1);
       
	uint32_t mask = (1 << (pin->port_pin & 0x1f));
	return *pins & mask;
}


static void lpc31xx_gpio_set_inp(const struct extcon_pin *pin)
{

volatile uint32_t  * mode0_reset = (uint32_t *)&IOCONF_EBI_MCI_M0_CLR + ((pin->port_pin & 0xFFE0)>>1);
volatile uint32_t * mode1_reset = (uint32_t *)&IOCONF_EBI_MCI_M1_CLR + ((pin->port_pin & 0xFFE0)>>1);
	uint32_t mask = (1 << (pin->port_pin & 0x1f));

	*mode0_reset = mask;
	*mode1_reset = mask;
}


static void lpc31xx_gpio_set_outp(const struct extcon_pin *pin, int val)
{
  
volatile uint32_t * mode0 = (uint32_t *)&IOCONF_EBI_MCI_M0 + ((pin->port_pin & 0xFFE0)>>1);
volatile uint32_t * mode1_set = (uint32_t *)&IOCONF_EBI_MCI_M1_SET + ((pin->port_pin & 0xFFE0)>>1);
	uint32_t mask = (1 << (pin->port_pin & 0x1f));

	*mode1_set = mask;

	if(val) {
		*mode0 |= mask;
	} else {
		*mode0 &= ~mask;
	}
}


static const struct extcon_pin extcon_main_outp = PIN(13, 7,11, 0);

static const struct extcon_pin extcon_pins[] = {
	/* CONN_PIN   PORT_PIN  CHIP_PIN */
	PIN(  4,	1, 2, 	0),
	PIN(  5,	1, 1, 	0),
	PIN(  6,	1, 0, 	0),
	PIN(  7, 10, 0, 	0),
	PIN(  8, 10, 3, 	0),
	PIN(  9, 10, 2, 	0),
	PIN( 10,	0,13, 	0),
	PIN( 11,	0,12, 	0),
	PIN( 14,	7,10, 	0),
	PIN( 15,	7, 9, 	0),
	PIN( 16,	7, 8, 	0),
	PIN( 17,	7, 7, 	0),
	PIN( 18,	7, 6, 	0),
	PIN( 19,	7, 5, 	0),
	PIN( 20,	7, 4, 	0),
	PIN( 21,	7, 3, 	0),
/*	PIN( 22,	2, 0, 	0),
	PIN( 23,	7, 6, 	0),
	PIN( 24,	7, 5, 	0),
	PIN( 25,	7, 6, 	0),
	PIN( 26,	7, 5, 	0),
	PIN( 30,	7, 6, 	0),
	PIN( 31,	7, 6, 	0),*/
	PIN( 32,	3, 1, 	0),
	PIN( 33,	3, 0, 	0),
	PIN( 34,	3, 2, 	0),
	PIN( 35,	1, 5, 	0),
	PIN( 36,	0, 7, 	0),
	PIN( 37,	1, 6, 	0),
	PIN( 38,	0, 6, 	0),
/*	PIN( 39,	7, 5, 	0),
	PIN( 40,	7, 6, 	0),
	PIN( 41,	7, 6, 	0),
	PIN( 42,	7, 5, 	0),*/
	PIN( 43, 11, 0, 	0),
	PIN( 44,	9, 2, 	0),
	PIN( 45,	9, 1, 	0),
	PIN( 47,	9, 3, 	0),
	PIN( 48,	9, 4, 	0),
/*	PIN( 49, 12, 1, 	0),
	PIN( 51,	1, 4, 	0),*/
};





static const struct extcon_jack extcons[] = {
	{
		.name = "EXT",
		.main_outp_pin = &extcon_main_outp,
		.pins = extcon_pins,
		.num_pins = ARRAY_NUMELEM(extcon_pins)
	},
};

static void setup_delay(void)
{
	for(volatile int i=0; i<1000; i++);
}

const struct extcon_board extcon_board_def = {
	.name = "OLIMEX LPC-H3131",
	.connectors = extcons,
	.num_connectors = ARRAY_NUMELEM(extcons),

	.gpio_read = lpc31xx_gpio_read,
	.gpio_set_inp = lpc31xx_gpio_set_inp,
	.gpio_set_outp = lpc31xx_gpio_set_outp,

	.print_port_pin = print_port_pin,
	.print_con_pin = 0,	/* use default implementation */
	.print_chip_pin = 0,	/* use default implementation */
	
	.setup_delay = setup_delay,
	
	.puts = _puts,
};



