/*
 * LWCloneU2 – Laufzeit-Konfiguration / runtime configuration
 *
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

#include <string.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include <hwconfig.h>
#include "config.h"
#include "pins.h"
#include "keydefs.h"


config_t g_cfg;

/* Feste EEPROM-Adresse, damit die Konfiguration Firmware-Updates überlebt. Die ersten 64 Byte
 * bleiben für die LED-Wiz-ID (main_usb.c, EEMEM) frei.
 * Fixed EEPROM address so the configuration survives firmware updates. The first 64 bytes stay
 * free for the LED-Wiz id (main_usb.c, EEMEM). */
#define EE_CONFIG_ADDR  ((void *)64)

_Static_assert(64 + sizeof(config_t) <= E2END + 1, "config_t passt nicht ins EEPROM / config_t does not fit into EEPROM");
_Static_assert(sizeof(config_t) == 20 + 4 * CFG_NUM_OUTPUTS + 4 * CFG_NUM_INPUTS + 12 * CFG_NUM_AXES,
               "config_t Layout geaendert – Tool anpassen / layout changed – adapt the tool");


uint16_t config_crc16(const uint8_t *p, uint16_t n)
{
	/* CRC-16/CCITT-FALSE, Startwert 0xFFFF, Polynom 0x1021 */
	uint16_t crc = 0xFFFF;

	while (n--)
	{
		crc ^= (uint16_t)(*p++) << 8;

		for (uint8_t i = 0; i < 8; i++)
			crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
	}

	return crc;
}

static uint16_t cfg_body_crc(const config_t *cfg)
{
	return config_crc16((const uint8_t *)cfg + 8, (uint16_t)(sizeof(*cfg) - 8));
}

static bool pin_ok(uint8_t pin)
{
	pin_io_t io;
	return pin == CFG_PIN_NONE || pins_resolve(pin, &io);   /* ungültig oder gesperrt → false */
}

/* Jeder Pin darf nur einmal belegt sein / every pin may be used only once */
static bool pin_claim(uint8_t *used, uint8_t pin)
{
	if (pin == CFG_PIN_NONE)
		return true;

	uint8_t const byte = pin >> 3;
	uint8_t const bit  = (uint8_t)(1 << (pin & 7));

	if (used[byte] & bit)
		return false;

	used[byte] |= bit;
	return true;
}

bool config_validate(const config_t *cfg)
{
	uint8_t used[32];   /* Bitmaske für bis zu 254 Pins / bit mask for up to 254 pins */
	memset(used, 0x00, sizeof(used));

	if (cfg->magic[0] != CFG_MAGIC0 || cfg->magic[1] != CFG_MAGIC1)
		return false;

	if (cfg->version != CFG_VERSION || cfg->size != sizeof(config_t) || cfg->board != BOARD_ID)
		return false;

	if (cfg->n_out != CFG_NUM_OUTPUTS || cfg->n_in != CFG_NUM_INPUTS || cfg->n_axes != CFG_NUM_AXES)
		return false;

	if (!pin_ok(cfg->ws.pin) || cfg->ws.count > WS2812_MAX_PIXELS || cfg->ws.order > WS_ORDER_BRG)
		return false;

	if (!pin_claim(used, cfg->ws.pin))
		return false;

	for (uint8_t i = 0; i < CFG_NUM_OUTPUTS; i++)
	{
		const cfg_output_t *o = &cfg->out[i];

		switch (o->type)
		{
		case OUT_NONE:
			break;

		case OUT_GPIO:
			if (!pin_ok(o->a) || !pin_claim(used, o->a))
				return false;
			break;

		case OUT_WS2812:
			if (o->b == 0 || (uint16_t)o->a + o->b > cfg->ws.count || o->c > WS_CH_WHITE)
				return false;
			break;

		default:
			return false;
		}
	}

	for (uint8_t i = 0; i < CFG_NUM_INPUTS; i++)
	{
		if (!pin_ok(cfg->in[i].pin) || !pin_claim(used, cfg->in[i].pin))
			return false;
	}

	for (uint8_t i = 0; i < CFG_NUM_AXES; i++)
	{
		const cfg_axis_t *a = &cfg->axes[i];

		if (a->pin == CFG_PIN_NONE)
			continue;

		if (!pin_ok(a->pin) || !pins_has_adc(a->pin) || !pin_claim(used, a->pin))
			return false;

		if (a->joy >= MAX_JOYSTICKS || a->axis >= NUM_AXES_PER_JOY)
			return false;

		if (a->min > 1023 || a->max > 1023 || a->center > 1023 || a->deadzone > 127)
			return false;

		if (a->min >= a->center || a->center >= a->max)
			return false;
	}

	return true;
}

void config_load_defaults(void)
{
	memset(&g_cfg, 0x00, sizeof(g_cfg));

	g_cfg.magic[0] = CFG_MAGIC0;
	g_cfg.magic[1] = CFG_MAGIC1;
	g_cfg.version  = CFG_VERSION;
	g_cfg.board    = BOARD_ID;
	g_cfg.size     = sizeof(config_t);
	g_cfg.n_out    = CFG_NUM_OUTPUTS;
	g_cfg.n_in     = CFG_NUM_INPUTS;
	g_cfg.n_axes   = CFG_NUM_AXES;

	g_cfg.ws.pin   = CFG_PIN_NONE;
	g_cfg.ws.count = 0;
	g_cfg.ws.order = WS_ORDER_GRB;

	for (uint8_t i = 0; i < CFG_NUM_OUTPUTS; i++)
	{
		g_cfg.out[i].type = OUT_NONE;
		g_cfg.out[i].a    = CFG_PIN_NONE;
	}

	for (uint8_t i = 0; i < CFG_NUM_INPUTS; i++)
		g_cfg.in[i].pin = CFG_PIN_NONE;

	for (uint8_t i = 0; i < CFG_NUM_AXES; i++)
	{
		g_cfg.axes[i].pin      = CFG_PIN_NONE;
		g_cfg.axes[i].min      = 0;
		g_cfg.axes[i].center   = 512;
		g_cfg.axes[i].max      = 1023;
		g_cfg.axes[i].deadzone = 8;
	}

	/* Board-spezifische Vorbelegung (pinmap.h) / board specific defaults */
	#if defined(BOARD_DEFAULT_OUTPUTS)
	{
		static const uint8_t PROGMEM defout[] = { BOARD_DEFAULT_OUTPUTS };
		uint8_t n = sizeof(defout);
		if (n > CFG_NUM_OUTPUTS) n = CFG_NUM_OUTPUTS;

		for (uint8_t i = 0; i < n; i++)
		{
			uint8_t const pin = pgm_read_byte(&defout[i]);
			if (pin == CFG_PIN_NONE) continue;
			g_cfg.out[i].type = OUT_GPIO;
			g_cfg.out[i].a    = pin;
			g_cfg.out[i].b    = 0;
		}
	}
	#endif

	#if defined(BOARD_DEFAULT_INPUTS)
	{
		/* Paare (pin, func) / pairs (pin, func) */
		static const uint8_t PROGMEM defin[] = { BOARD_DEFAULT_INPUTS };
		uint8_t n = sizeof(defin) / 2;
		if (n > CFG_NUM_INPUTS) n = CFG_NUM_INPUTS;

		for (uint8_t i = 0; i < n; i++)
		{
			g_cfg.in[i].pin  = pgm_read_byte(&defin[2 * i]);
			g_cfg.in[i].func = pgm_read_byte(&defin[2 * i + 1]);
		}
	}
	#endif
}

bool config_save(void)
{
	if (!config_validate(&g_cfg))
		return false;

	g_cfg.crc = cfg_body_crc(&g_cfg);
	eeprom_update_block(&g_cfg, EE_CONFIG_ADDR, sizeof(g_cfg));

	return true;
}

void config_init(void)
{
	eeprom_read_block(&g_cfg, EE_CONFIG_ADDR, sizeof(g_cfg));

	if (!config_validate(&g_cfg) || g_cfg.crc != cfg_body_crc(&g_cfg))
	{
		config_load_defaults();
	}
}
