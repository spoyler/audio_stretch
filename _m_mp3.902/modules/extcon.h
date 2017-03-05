/****************************************************************************
 *
 * Project: Generic test routines.
 *
 * Copyright by Olimex Ltd. All rights reserved.
 *
 * File: extcon.h
 * Description: External connector test.
 * Developer: Dimitar Dimitrov ( dinuxbg,gmail.com )
 *
 * Last change: $Date: 2009/05/15 $
 * Revision: $Revision: #1 $
 * Id: $Id: //depot/IAR/auxiliary_arm2_5_xx/TestCode/NXP/LPC17xx/IAR-LPC-1766-SK/app/extcon.h#1 $
 *
 ****************************************************************************/

#ifndef EXTCON_H
#define EXTCON_H

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#ifndef EIO
  #define EIO	5
#endif

/** Maximal chip/port/connector pin name length. */
#define EXTCON_PIN_NAME_MAXLEN	16

/** Holds information about a connector pin. */
struct extcon_pin {
	/** Position of the pin in the connector header. */
	int con_pin;

	/**
	 * Chips usually group several GPIOs into ports. This holds
	 * information about both the port and the pin's port bit on the chip. 
	 */
	int port_pin;

	/** Physical pin number on the chip package. */
	int chip_pin;
};


/** Defines a single connector (jack/header). */
struct extcon_jack {
	/** Connector name. */
	const char *name;

	/** Pin with the low resistor on the test board (typical 1k). */
	const struct extcon_pin *main_outp_pin;

	/** An array of the rest of the connector GPIO pins, which have
	 * a larger resistor (typical 10k). */
	const struct extcon_pin *pins;

	/** Number of elements in @a pins. */
	const unsigned int num_pins;
};

/**
 * Board definition, containing all board connectors and board-specific
 * stuff.
 */
struct extcon_board {
	/** Board name. */
	const char *name;

	/** An array of all board connectors/jacks/headers. */
	const struct extcon_jack *connectors;

	/** Number of elements in @a connectors. */
	const unsigned int num_connectors;

	/** Function for reading the actual value on a GPIO pin. */	
	int (*gpio_read)(const struct extcon_pin *pin);

	/** Function for configuring a GPIO as an input. */
	void (*gpio_set_inp)(const struct extcon_pin *pin);

	/** Function for configuring a GPIO as an output with a 
	 * defined output value. */
	void (*gpio_set_outp)(const struct extcon_pin *pin, int val);

	/**
	 * Called for delaying execution enough for settling of the
	 * output pin values.
	 * @note A default implementation will be called if this is NULL.
	 */
	void (*setup_delay)(void);

	/** 
	 * Print pin's name in a GPIO port perspective, e.g. "PA5".
	 * @note A default implementation will be called if this is NULL.
	 */
	void (*print_port_pin)(char *buf, const struct extcon_pin *pin);

	/** 
	 * Print pin's name in a connector perspective, e.g. "5".
	 * @note A default implementation will be called if this is NULL.
	 */
	void (*print_con_pin)(char *buf, const struct extcon_pin *pin);

	/** 
	 * Print pin's name in a chip perspective, e.g. "R3" for BGA
	 * packages or "5" for TQFP/DIP/QFN packages.
	 * @note A default implementation will be called if this is NULL.
	 */
	void (*print_chip_pin)(char *buf, const struct extcon_pin *pin);

	/**
	 * Function for outputting message strings.
	 * @note STDIO's puts will be called if this is NULL.
	 */
	void (*puts)(const char *s);
};

/**
 * Perform a test on all board's connectors.
 */
extern int extcon_boardtest(const struct extcon_board *brd);


#endif	/* EXT_CON_H */

