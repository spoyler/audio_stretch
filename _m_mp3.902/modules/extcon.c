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
 * Id: $Id: //depot/IAR/auxiliary_arm2_5_xx/TestCode/NXP/LPC17xx/IAR-LPC-1766-SK/app/extcon.c#1 $
 *
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "extcon.h"

/** Utility operator for iterating through connector's pins. */
#define FOR_EACH_PIN(pin, con)	\
		for(	(pin) = (con)->pins; \
			(pin) != ((con)->pins + (con)->num_pins); \
			(pin)++)

/** Utility operator for iterating through board's connectors. */
#define FOR_EACH_CONNECTOR(con, brd)	\
		for(	(con) = (brd)->connectors; \
			(con) != ((brd)->connectors + (brd)->num_connectors); \
			(con)++)

#define CONFIG_MAX_FAILS	10

static char charbuf[512];

static void print(const char *msg, const struct extcon_board *brd)
{
	if(brd->puts) {
		brd->puts(msg);
	} else {
		puts(msg);
	}
}


/**
 * Print a port pin name using board's callbacks or, if the former is not 
 * present, using a default implementation.
 */
static void print_port_pin(char *buf, const struct extcon_pin *pin, const struct extcon_board *brd)
{
	if(brd->print_port_pin) {
		brd->print_port_pin(buf, pin);
	} else {
		sprintf(buf, "%d", pin->port_pin);
	}
}


/**
 * Print a connector pin name using board's callbacks or, if the former is not 
 * present, using a default implementation.
 */
static void print_con_pin(char *buf, const struct extcon_pin *pin, const struct extcon_board *brd)
{
	if(brd->print_con_pin) {
		brd->print_con_pin(buf, pin);
	} else {
		sprintf(buf, "%d", pin->con_pin);
	}
}


/**
 * Print a chip pin name using board's callbacks or, if the former is not 
 * present, using a default implementation.
 */
static void print_chip_pin(char *buf, const struct extcon_pin *pin, const struct extcon_board *brd)
{
	if(brd->print_chip_pin) {
		brd->print_chip_pin(buf, pin);
	} else if(pin->chip_pin < 0) {
		sprintf(buf, "<undef>");
	} else {
		sprintf(buf, "%d", pin->chip_pin);
	}
}


/** Provide a sufficient time for pin output settling. */
static void setup_delay(const struct extcon_board *brd) 
{
	if(brd->setup_delay) 
		brd->setup_delay();
	else {
		volatile int i = 1000;
		while(i--);
	}
}


/**
 * Print a message indicating a fault pin. Error context can be given in
 * the @a errmsg parameter.
 */
static void pin_fault(
		const struct extcon_board *brd,
		const struct extcon_jack *con,
		const struct extcon_pin *pin,
		const char *errmsg)
{
	char con_pin_name[EXTCON_PIN_NAME_MAXLEN];
	char chip_pin_name[EXTCON_PIN_NAME_MAXLEN];
	char port_pin_name[EXTCON_PIN_NAME_MAXLEN];

	print_con_pin(con_pin_name, pin, brd);
	print_chip_pin(chip_pin_name, pin, brd);
	print_port_pin(port_pin_name, pin, brd);

	sprintf( charbuf,
		"---------------------------------------------------------\n\r"
		"ERROR:  %s\n\r"
		"        Board %s, connector %s\n\r"
		"        connector pin: %s\n\r"
		"        chip pin: %s\n\r"
		"        port pin: %s\n\r"
		"---------------------------------------------------------\n\r",
		errmsg ? errmsg : "(generic) pin fault",
		brd->name, con->name,
		con_pin_name, 
		chip_pin_name,
		port_pin_name);

	print(charbuf, brd);
}


static void pin_short_circuit(
		const struct extcon_board *brd,
		const struct extcon_jack *con,
		const struct extcon_pin *pin1,
		const struct extcon_pin *pin2)
{
	char con_pin1_name[EXTCON_PIN_NAME_MAXLEN];
	char chip_pin1_name[EXTCON_PIN_NAME_MAXLEN];
	char port_pin1_name[EXTCON_PIN_NAME_MAXLEN];
	char con_pin2_name[EXTCON_PIN_NAME_MAXLEN];
	char chip_pin2_name[EXTCON_PIN_NAME_MAXLEN];
	char port_pin2_name[EXTCON_PIN_NAME_MAXLEN];


	print_con_pin(con_pin1_name, pin1, brd);
	print_chip_pin(chip_pin1_name, pin1, brd);
	print_port_pin(port_pin1_name, pin1, brd);
	print_con_pin(con_pin2_name, pin2, brd);
	print_chip_pin(chip_pin2_name, pin2, brd);
	print_port_pin(port_pin2_name, pin2, brd);


	sprintf( charbuf,
		"---------------------------------------------------------\n\r"
		"ERROR:  short circuit\n\r"
		"        Board %s, connector %s\n\r"
		"        connector pins: %s, %s\n\r"
		"        chip pins: %s, %s\n\r"
		"        port pins: %s, %s\n\r"
		"---------------------------------------------------------\n\r",
		brd->name, con->name,
		con_pin1_name, con_pin2_name,
		chip_pin1_name, chip_pin2_name,
		port_pin1_name, port_pin2_name);

	print(charbuf, brd);
}


/**
 * Test a particular connector.
 */
static int test_connector(
		const struct extcon_board *brd, 
		const struct extcon_jack *con, 
		const int polarity)
{
	const struct extcon_pin *pin, *outp_pin;
	int nfails = 0;

	/*-----------------------------------------------------------------*/

	/* at first the main pin is output, the other are inputs */
	brd->gpio_set_outp(con->main_outp_pin, polarity);

	FOR_EACH_PIN(pin, con) {
		brd->gpio_set_inp(pin);
	}
	
	if(!!brd->gpio_read(con->main_outp_pin) != polarity) {
		pin_fault(brd, con, con->main_outp_pin, 
			  polarity ? 	
				  "pin cannot be set to a high value"
				: "pin cannot be set to a low value");
		nfails++;
	}

	/*-----------------------------------------------------------------*/

	/* check whether pins can sense a value from the main output */
	brd->gpio_set_outp(con->main_outp_pin, polarity);
	setup_delay(brd);

	FOR_EACH_PIN(pin, con) {
		if(!!brd->gpio_read(pin) != polarity) {
			pin_fault(brd,con,pin,
				polarity ? 
					  "pin cannot sense a high value"
					: "pin cannot sense a low value");
			nfails++;
		}
	}
	
	/*-----------------------------------------------------------------*/
#if 0	/* optional test that relies on specific extension connectors */
	/* check whether the main output can sense values from other pins */
	brd->gpio_set_inp(con->main_outp_pin);

	FOR_EACH_PIN(pin, con) {
		int val = rand() % 2;
		
		brd->gpio_set_outp(pin, val);
		setup_delay(brd);

		if(!!brd->gpio_read(con->main_outp_pin) != val) {
			pin_fault(brd,con,con->main_outp_pin,
				val ? 
					  "pin cannot sense a high value"
					: "pin cannot sense a low value");
			nfails++;
		}
		
		brd->gpio_set_inp(pin);
	}
#endif
	/*-----------------------------------------------------------------*/
	
	/* check for short circuits between connector pins */
	brd->gpio_set_outp(con->main_outp_pin, !polarity);
	setup_delay(brd);

	FOR_EACH_PIN(outp_pin, con) {
		if(nfails > CONFIG_MAX_FAILS)
			break;

		brd->gpio_set_outp(outp_pin, polarity);
		setup_delay(brd);
		
		if(!!brd->gpio_read(con->main_outp_pin) != !polarity) {
			pin_fault(brd, con, con->main_outp_pin,
				!polarity ?
					  "pin cannot be set to a high value"
					: "pin cannot be set to a low value");
			nfails++;
		}

		FOR_EACH_PIN(pin, con) {
			if(pin == outp_pin) {
				if(!!brd->gpio_read(pin) != polarity) {
					pin_fault(brd, con, pin,
						polarity ?
							"pin cannot be set"
							" to a high value"
							:
							"pin cannot be set"
							" to a low value"
							);
					nfails++;
				}
			} else {
				if(!!brd->gpio_read(pin) != !polarity) {
					pin_short_circuit(brd, con, pin, outp_pin);
					nfails++;
				}
			}
		}

		brd->gpio_set_inp(outp_pin);
	}

	return nfails;
}


int extcon_boardtest(const struct extcon_board *brd)
{
	const struct extcon_jack *con;
	int stat = 0;

	/* some sanity checks */
	assert(brd->name);
	assert(brd->connectors);
	assert(brd->gpio_read);
	assert(brd->gpio_set_inp);
	assert(brd->gpio_set_outp);

	sprintf(charbuf, "Testing the connectors of board %s...\n\r", brd->name);
	print(charbuf, brd);

	/* first set all connector pins as outputs */
	FOR_EACH_CONNECTOR(con, brd) {
		const struct extcon_pin *pin;

		brd->gpio_set_outp(con->main_outp_pin, 1);
		FOR_EACH_PIN(pin, con) {
			brd->gpio_set_outp(pin, 1);
		}
	}

	/* test connectors */
	FOR_EACH_CONNECTOR(con, brd) {
		int nfails;

		sprintf(charbuf, "Testing connector %s...\n\r", con->name);
		print(charbuf, brd);

		nfails = test_connector(brd, con, 1);
		nfails += test_connector(brd, con, 0);

		if(nfails) {
			sprintf(charbuf,"Connector %s FAULT !!!\n\r",con->name);
			print(charbuf, brd);

			sprintf(charbuf, "A total of %d number of failed "
					"individual pin tests.\n\r", nfails);
			print(charbuf, brd);
			stat = -EIO;
		} else {
			sprintf(charbuf, "Connector %s OK\n\r", con->name);
			print(charbuf, brd);
		}
		if(nfails > CONFIG_MAX_FAILS)
			break;
	}

	if(stat) {
		sprintf(charbuf, "ERROR: Board %s connector test FAILED!!!\n\r", 
				brd->name);
		print(charbuf, brd);
	} else {
		sprintf(charbuf, "Board %s connectors OK\n\r", brd->name);
		print(charbuf, brd);
	}

	return stat;
}

