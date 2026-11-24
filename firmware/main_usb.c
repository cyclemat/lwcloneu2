/*
 * LWCloneU2 – Hauptprogramm des USB-MCUs / main program of the USB MCU
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
 * Zwei Betriebsarten / two modes:
 *
 *  - Ein-Chip (Leonardo, Pro Micro, Breakout 32U2): Dieser MCU hat USB *und* die Pins.
 *    Reports gehen direkt an proto.c, Antworten und Panel-Reports kommen direkt von hier.
 *
 *  - Brücke (ATmega16U2/8U2 auf Mega/Uno, DATA_TX_UART_vect definiert): Dieser MCU hat nur USB.
 *    8-Byte-Reports werden über die UART an den Haupt-MCU (main_led.c) weitergereicht; von dort
 *    kommen Panel-Reports (Byte 0 = Report-ID 1..7) und Konfigurations-Antworten (Byte 0 >= 0xA0),
 *    die hier auf den jeweiligen Endpunkt gelegt werden.
 *
 *  Die LED-Wiz-ID (SETID) wird immer hier gespeichert, denn sie gehört zum USB-Deskriptor.
 *  The LED-Wiz id (SETID) is always stored here because it belongs to the USB descriptor.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <LUFA/Drivers/USB/USB.h>
#include "descriptors.h"

#include <hwconfig.h>
#include "comm.h"
#include "clock.h"
#include "panel.h"
#include "proto.h"

#if defined(DATA_TX_UART_vect)
	#define USB_BRIDGE 1
#else
	#include "config.h"
	#include "outputs.h"
#endif


#define LWC_CONFIG_IDENTIFIER 0xA62817B2   // some magic number(s)
#define RESETSTATE_BOOTLOADER 0x42B8217C

#define OFFSET_OF(_struct_, _member_) (((uint8_t*)&(((_struct_*)NULL)->_member_)) - (uint8_t*)NULL)


uint32_t g_resetstate __attribute__ ((section (".noinit")));

typedef struct {
	uint32_t id;
	uint8_t ledwiz_id;
	uint8_t reserved[7];
} lwc_config_t;

/* liegt am EEPROM-Anfang; die Laufzeit-Konfiguration (config.c) beginnt bei Adresse 64
 * lives at the start of the EEPROM; the runtime configuration (config.c) starts at address 64 */
static struct {
	uint8_t reserved[16];
	uint8_t configdata[sizeof(lwc_config_t)];
} g_eeprom_table EEMEM;


static void hardware_init(void);
static void main_task(void);
static void hardware_restart(bool enter_bootloader);
static void configure_device(void);
static void handle_output_report(uint8_t *p8);


int main(void)
{
	hardware_init();
	USB_Init();

	sei();

	DbgOut(DBGINFO, "enter main loop");

	for (;;)
	{
		USB_USBTask();
		main_task();
		sleep_ms(0);
	}
}


static void hardware_init(void)
{
	uint8_t const mcusr = MCUSR; // save status register

	// Disable watchdog if enabled by bootloader/fuses
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	#if defined(BOOTLOADER_START_ADDR)
	// if that was a reset due to watchdog timeout that we issued, call the bootloader
	if ((mcusr & (1 << WDRF)) &&
	    (g_resetstate == RESETSTATE_BOOTLOADER))
	{
		g_resetstate = 0;
		void (*bootloader)(void) = (void*)(BOOTLOADER_START_ADDR);
		bootloader();
	}
	#else
	(void)mcusr;
	#endif

	// Disable clock division
	clock_prescale_set(clock_div_1);

	set_sleep_mode(SLEEP_MODE_IDLE);

	clock_init();
	comm_init();

	#if !defined(USB_BRIDGE)
	config_init();
	outputs_init();
	panel_init();
	#endif

	configure_device();
}


// load current configuration from eeprom and set USB product ID

static void configure_device(void)
{
	lwc_config_t cfg;
	eeprom_read_block((void*)&cfg, g_eeprom_table.configdata, sizeof(cfg));

	if (cfg.id != LWC_CONFIG_IDENTIFIER)
	{
		memset(&cfg, 0x00, sizeof(cfg));
		cfg.id = LWC_CONFIG_IDENTIFIER;
		cfg.ledwiz_id = USB_PRODUCT_ID & 0x0F;

		eeprom_write_block((void*)&cfg, g_eeprom_table.configdata, sizeof(cfg));
	}

	// set USB product ID
	uint16_t const product_id = (USB_PRODUCT_ID & ~0x000F) | (cfg.ledwiz_id & 0x0F);
	SetProductID(product_id);
}


/* 64-Byte-Input-Report auf dem LED-Endpunkt: 8 Byte Nutzdaten + Nullen
 * 64 byte input report on the LED endpoint: 8 bytes payload + zeros */
static bool send_led_reply(const uint8_t *p8)
{
	#if defined(ENABLE_LED_DEVICE)
	Endpoint_SelectEndpoint(LED_EPADDR);

	if (!Endpoint_IsINReady())
		return false;

	Endpoint_Write_Stream_LE(p8, PROTO_REPORT_SIZE, NULL);
	Endpoint_Null_Stream(LED_EPSIZE - PROTO_REPORT_SIZE, NULL);
	Endpoint_ClearIN();
	return true;
	#else
	(void)p8;
	return true;
	#endif
}

static bool send_panel_report(const uint8_t *pdata, uint8_t ndata)
{
	#if defined(ENABLE_PANEL_DEVICE)
	Endpoint_SelectEndpoint(PANEL_EPADDR);

	if (!Endpoint_IsINReady())
		return false;

	Endpoint_Write_Stream_LE(pdata, ndata, NULL);
	Endpoint_ClearIN();
	return true;
	#else
	(void)pdata; (void)ndata;
	return true;
	#endif
}


static void main_task(void)
{
	#if !defined(USB_BRIDGE)
	outputs_task();
	#endif

	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	#if defined(USB_BRIDGE)

	/* Nachrichten vom Haupt-MCU / messages from the main MCU */
	msg_t * const pmsg = msg_recv();

	if (pmsg != NULL)
	{
		bool done = true;

		if (pmsg->nlen < 2 || pmsg->nlen > 8)
		{
			DbgOut(DBGERROR, "main_usb, invalid framesize");
		}
		else if (proto_is_cfg_cmd(pmsg->data[0]))
		{
			uint8_t r[PROTO_REPORT_SIZE];
			memset(r, 0x00, sizeof(r));
			memcpy(r, &pmsg->data[0], pmsg->nlen);
			done = send_led_reply(r);
		}
		else
		{
			done = send_panel_report(&pmsg->data[0], pmsg->nlen);
		}

		if (done)
			msg_release();     /* sonst im nächsten Durchlauf erneut versuchen / else retry next time */
	}

	#else

	/* Konfigurations-Antworten / configuration replies */
	{
		#if defined(ENABLE_LED_DEVICE)
		Endpoint_SelectEndpoint(LED_EPADDR);

		if (Endpoint_IsINReady())
		{
			uint8_t r[PROTO_REPORT_SIZE];

			if (proto_pop_reply(r))
				send_led_reply(r);
		}
		#endif
	}

	/* Panel-Reports / panel reports */
	{
		#if defined(ENABLE_PANEL_DEVICE)
		Endpoint_SelectEndpoint(PANEL_EPADDR);

		if (Endpoint_IsINReady())
		{
			uint8_t * pdata = NULL;
			uint8_t const ndata = panel_get_report(&pdata);

			if (ndata > 0)
				send_panel_report(pdata, ndata);
		}
		#endif
	}

	#endif
}


void EVENT_USB_Device_Connect(void)
{
}

void EVENT_USB_Device_Disconnect(void)
{
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	Endpoint_ConfigureEndpoint(MISC_EPADDR, EP_TYPE_INTERRUPT, MISC_EPSIZE, 1);
	#if defined(ENABLE_LED_DEVICE)
	Endpoint_ConfigureEndpoint(LED_EPADDR, EP_TYPE_INTERRUPT, LED_EPSIZE, 1);
	#endif
	#if defined(ENABLE_PANEL_DEVICE)
	Endpoint_ConfigureEndpoint(PANEL_EPADDR, EP_TYPE_INTERRUPT, PANEL_EPSIZE, 1);
	#endif
}

void EVENT_USB_Device_ControlRequest(void)
{
	switch (USB_ControlRequest.bRequest)
	{
	case HID_REQ_GetReport:
		if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
		{
			Endpoint_ClearSETUP();

			uint8_t zero = 0;

			// Write one 'zero' byte report data to the control endpoint
			Endpoint_Write_Control_Stream_LE(&zero, 1);
			Endpoint_ClearOUT();
		}
		break;

	case HID_REQ_SetReport:
		if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
		{
			Endpoint_ClearSETUP();

			uint8_t data[PROTO_REPORT_SIZE];
			memset(data, 0x00, sizeof(data));

			uint16_t const nlen = (USB_ControlRequest.wLength < sizeof(data)) ? USB_ControlRequest.wLength : sizeof(data);

			Endpoint_Read_Control_Stream_LE(data, nlen);
			Endpoint_ClearIN();

			DbgOut(DBGINFO, "SetReport: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
				data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

			handle_output_report(data);
		}
		break;
	}
}


static void handle_output_report(uint8_t *p8)
{
	/* Befehle, die den USB-MCU betreffen / commands that concern the USB MCU */
	if (p8[0] == CMD_SETID || p8[0] == CMD_DFU)
	{
		const uint8_t id = p8[1];
		const uint8_t check = (uint8_t)~id;

		if (p8[2] == 0xFF && p8[3] == 0xFF && p8[4] == 0xFF && p8[5] == 0xFF && p8[6] == 0xFF && p8[7] == check)
		{
			if (p8[0] == CMD_SETID)
			{
				eeprom_update_byte(
					&g_eeprom_table.configdata[0] + OFFSET_OF(lwc_config_t, ledwiz_id),
					id & 0x0F);

				hardware_restart(false);
			}
			else
			{
				#if defined(BOOTLOADER_START_ADDR)
				hardware_restart(true);
				#endif
			}
		}

		return;
	}

	#if defined(USB_BRIDGE)

	msg_t * const pmsg = msg_prepare();

	if (pmsg == NULL)
	{
		DbgOut(DBGERROR, "SetReport: tx buffer full");
		return;
	}

	memcpy(&pmsg->data[0], p8, PROTO_REPORT_SIZE);
	pmsg->nlen = PROTO_REPORT_SIZE;
	msg_send();

	#else

	proto_handle_report(p8);

	#endif
}


static void hardware_restart(bool enter_bootloader)
{
	// detach from the bus
	USB_Disable();
	USB_Detach();

	// Disable all interrupts
	cli();

	// Wait some time for the USB detachment to register on the host
	_delay_ms(250);

	// force a reset via watchdog timeout
	wdt_enable(WDTO_15MS);

	// set persistant variable, so we know that we issued the reset
	g_resetstate = enter_bootloader ? RESETSTATE_BOOTLOADER : 0;

	for (;;) {;}
}
