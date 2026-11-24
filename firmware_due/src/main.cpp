/*
 * LWCloneU2 – Hauptprogramm Arduino Due (SAM3X8E, nativer USB-Port)
 * LWCloneU2 – main program Arduino Due (SAM3X8E, native USB port)
 *
 * Copyright (C) 2013 Andreas Dittrich <lwcloneu2@cithraidt.de> (Ablauf nach main_usb.c)
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
 * Ein-Chip-Betrieb wie Leonardo/Pro Micro: dieser MCU hat USB und alle Pins. Die Logik (LED-Wiz-Protokoll,
 * Konfiguration, Soft-PWM, Panel, Strip, DOF-COM-Port) ist dieselbe wie bei den AVR-Boards (../firmware),
 * hier liegen nur USB-Anbindung und Hardware-Zugriff. Die LED-Wiz-ID (SETID) liegt wie beim AVR am Anfang
 * des (emulierten) EEPROMs, die Laufzeit-Konfiguration ab Offset 64 (config.c).
 *
 * Single-chip operation like Leonardo/Pro Micro: this MCU has USB and all pins. The logic (LED-Wiz
 * protocol, configuration, soft PWM, panel, strip, DOF COM port) is the same as on the AVR boards
 * (../firmware); only the USB glue and hardware access live here. The LED-Wiz id (SETID) sits at the
 * start of the (emulated) EEPROM like on AVR, the runtime configuration from offset 64 (config.c).
 */

#include <Arduino.h>
#include <string.h>
#include <stddef.h>
#include <avr/eeprom.h>
#include "lwc_shared.h"
#include "usb_lwz.h"
#include "hw_due.h"


#define LWC_CONFIG_IDENTIFIER 0xA62817B2
#define EE_ID_ADDR            ((void *)16)

typedef struct {
	uint32_t id;
	uint8_t  ledwiz_id;
	uint8_t  reserved[7];
} lwc_config_t;


/* LED-Wiz-ID laden (Werk: 0 → PID 0x00F0) / load the LED-Wiz id (factory: 0 → PID 0x00F0) */
static uint8_t load_ledwiz_id(void)
{
	lwc_config_t cfg;
	eeprom_read_block(&cfg, EE_ID_ADDR, sizeof(cfg));

	if (cfg.id != LWC_CONFIG_IDENTIFIER)
	{
		memset(&cfg, 0x00, sizeof(cfg));
		cfg.id = LWC_CONFIG_IDENTIFIER;
		cfg.ledwiz_id = 0;
		eeprom_write_block(&cfg, EE_ID_ADDR, sizeof(cfg));
	}

	return cfg.ledwiz_id & 0x0F;
}


/* Läuft im Core vor USBDevice.attach() – die PID muss vor der ersten Deskriptor-Anfrage feststehen.
 * Runs in the core before USBDevice.attach() – the PID must be known before the first descriptor request. */
void initVariant(void)
{
	lwz_usb_set_id(load_ledwiz_id());
}


void setup(void)
{
	config_init();
	outputs_init();
	panel_init();
	strip_init();
}


/* SETID / DFU (66) betreffen das USB-Gerät selbst, alles andere geht an proto.c
 * SETID / DFU (66) concern the USB device itself, everything else goes to proto.c */
static void handle_output_report(uint8_t *p8)
{
	if (p8[0] == CMD_SETID || p8[0] == CMD_DFU)
	{
		uint8_t const id = p8[1];
		uint8_t const check = (uint8_t)~id;

		if (p8[2] == 0xFF && p8[3] == 0xFF && p8[4] == 0xFF && p8[5] == 0xFF && p8[6] == 0xFF && p8[7] == check)
		{
			if (p8[0] == CMD_SETID)
			{
				uint8_t const v = id & 0x0F;
				eeprom_update_byte((uint8_t *)EE_ID_ADDR + offsetof(lwc_config_t, ledwiz_id), v);
				hw_restart(false);
			}
			else
			{
				hw_restart(true);     /* SAM-BA-Bootloader (ROM) / ROM SAM-BA bootloader */
			}
		}

		return;
	}

	proto_handle_report(p8);
}


extern "C" void strip_serial_tx(const uint8_t *p, uint8_t n)
{
	lwz_cdc_send(p, n);
}


void loop(void)
{
	uint8_t r[PROTO_REPORT_SIZE];

	/* Befehle vom Host (SET_REPORT, aus dem USB-Interrupt eingereiht) / commands from the host */
	while (lwz_usb_pop_report(r))
		handle_output_report(r);

	strip_task();

	/* DOF-COM-Port: höchstens ein Block je Durchlauf, damit Panel und Strip weiterlaufen
	 * DOF COM port: at most one chunk per pass so panel and strip keep running */
	for (uint8_t n = 0; n < 64 && SerialUSB.available(); n++)
		strip_serial_rx((uint8_t)SerialUSB.read());

	if (!USBDevice.configured())
		return;

	/* Konfigurations-/Strip-Antworten: 8 Byte Nutzdaten in einem 64-Byte-Report / replies */
	if (lwz_usb_in_ready(LWZ_IF_LED) && proto_pop_reply(r))
	{
		uint8_t rep[LED_REPLY_SIZE];
		memset(rep, 0x00, sizeof(rep));
		memcpy(rep, r, PROTO_REPORT_SIZE);
		lwz_usb_send(LWZ_IF_LED, rep, sizeof(rep));
	}

	/* Panel-Reports / panel reports */
	if (lwz_usb_in_ready(LWZ_IF_PANEL))
	{
		uint8_t *pdata = NULL;
		uint8_t const ndata = panel_get_report(&pdata);

		if (ndata > 0)
			lwz_usb_send(LWZ_IF_PANEL, pdata, ndata);
	}
}
