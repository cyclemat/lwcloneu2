/*
 * LWCloneU2 – Hauptprogramm des Haupt-MCUs hinter einer USB-Brücke (ATmega2560 / ATmega328P)
 *
 * Copyright (C) 2013 Andreas Dittrich <lwcloneu2@cithraidt.de>
 * Copyright (C) 2026 cyclemat
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

/*
 * Empfängt 8-Byte-Reports über die UART vom USB-MCU (main_usb.c, Brückenmodus), besitzt die
 * Ausgänge, Eingänge und die Konfiguration, und schickt Panel-Reports sowie Konfigurations-
 * Antworten zurück.
 *
 * Receives 8 byte reports via UART from the USB MCU (main_usb.c, bridge mode), owns outputs,
 * inputs and the configuration, and sends panel reports and configuration replies back.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#include <hwconfig.h>
#include "comm.h"
#include "clock.h"
#include "config.h"
#include "outputs.h"
#include "panel.h"
#include "proto.h"


int main(void)
{
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	clock_init();
	comm_init();
	config_init();
	outputs_init();
	panel_init();

	set_sleep_mode(SLEEP_MODE_IDLE);

	sei();

	DbgOut(DBGINFO, "main_led, enter main loop");

	for (;;)
	{
		outputs_task();

		/* Reports vom USB-MCU / reports from the USB MCU */
		msg_t * const prxmsg = msg_recv();

		if (prxmsg != NULL)
		{
			if (prxmsg->nlen != PROTO_REPORT_SIZE)
			{
				DbgOut(DBGERROR, "main_led, invalid framesize");
			}
			else
			{
				proto_handle_report(&prxmsg->data[0]);
			}

			msg_release();
			continue;
		}

		/* Antworten und Panel-Reports zum USB-MCU / replies and panel reports to the USB MCU */
		msg_t * const ptxmsg = msg_prepare();

		if (ptxmsg != NULL)
		{
			if (proto_pop_reply(&ptxmsg->data[0]))
			{
				ptxmsg->nlen = PROTO_REPORT_SIZE;
				msg_send();
				continue;
			}

			uint8_t * pdata = NULL;
			uint8_t const ndata = panel_get_report(&pdata);

			if (ndata > 0)
			{
				memcpy(&ptxmsg->data[0], pdata, ndata);
				ptxmsg->nlen = ndata;
				msg_send();
				continue;
			}
		}
		else
		{
			DbgOut(DBGERROR, "main_led, tx buffer full");
		}

		// nichts zu tun → schlafen bis zum nächsten Interrupt / nothing to do → sleep until the next interrupt
		sleep_ms(0);
	}

	return 0;
}
