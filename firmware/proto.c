/*
 * LWCloneU2 – USB-Befehle (LED-Wiz + Konfiguration) / USB commands
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
#include <util/atomic.h>

#include <hwconfig.h>
#include "proto.h"
#include "config.h"
#include "pins.h"
#include "outputs.h"
#include "panel.h"


#define REPLY_QUEUE_LEN 4

static uint8_t s_replies[REPLY_QUEUE_LEN][PROTO_REPORT_SIZE];
static volatile uint8_t s_rpos = 0;
static volatile uint8_t s_wpos = 0;


static void push_reply(const uint8_t *p8)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint8_t const next = (uint8_t)((s_wpos + 1) % REPLY_QUEUE_LEN);

		if (next == s_rpos)
			return;      /* voll: Antwort verwerfen / full: drop reply */

		memcpy(s_replies[s_wpos], p8, PROTO_REPORT_SIZE);
		s_wpos = next;
	}
}

bool proto_pop_reply(uint8_t *dst8)
{
	bool res = false;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (s_rpos != s_wpos)
		{
			memcpy(dst8, s_replies[s_rpos], PROTO_REPORT_SIZE);
			s_rpos = (uint8_t)((s_rpos + 1) % REPLY_QUEUE_LEN);
			res = true;
		}
	}

	return res;
}

static void apply_all(void)
{
	outputs_apply_config();
	panel_apply_config();
}

static void handle_cfg(const uint8_t *p8)
{
	uint8_t r[PROTO_REPORT_SIZE];
	memset(r, 0x00, sizeof(r));
	r[0] = p8[0];

	switch (p8[0])
	{
	case CMD_CFG_INFO:
		r[1] = PROTO_VERSION;
		r[2] = BOARD_ID;
		r[3] = FW_VERSION_MAJOR;
		r[4] = FW_VERSION_MINOR;
		r[5] = (uint8_t)(sizeof(config_t) & 0xFF);
		r[6] = (uint8_t)(sizeof(config_t) >> 8);
		r[7] = pins_count();
		break;

	case CMD_CFG_READ:
	{
		uint16_t const off = (uint16_t)p8[1] | ((uint16_t)p8[2] << 8);
		r[1] = p8[1];
		r[2] = p8[2];

		uint8_t const * const src = (const uint8_t *)&g_cfg;

		for (uint8_t i = 0; i < 5; i++)
			r[3 + i] = (off + i < sizeof(config_t)) ? src[off + i] : 0;
		break;
	}

	case CMD_CFG_WRITE:
	{
		uint16_t const off = (uint16_t)p8[1] | ((uint16_t)p8[2] << 8);
		r[1] = p8[1];
		r[2] = p8[2];

		if (off >= sizeof(config_t))
		{
			r[3] = STATUS_INVALID;
			break;
		}

		uint8_t * const dst = (uint8_t *)&g_cfg;

		for (uint8_t i = 0; i < 5; i++)
		{
			if (off + i < sizeof(config_t))
				dst[off + i] = p8[3 + i];
		}

		/* Kopf immer korrekt halten / keep header consistent */
		g_cfg.magic[0] = CFG_MAGIC0;
		g_cfg.magic[1] = CFG_MAGIC1;
		g_cfg.version  = CFG_VERSION;
		g_cfg.board    = BOARD_ID;
		g_cfg.size     = sizeof(config_t);
		g_cfg.n_out    = CFG_NUM_OUTPUTS;
		g_cfg.n_in     = CFG_NUM_INPUTS;
		g_cfg.n_axes   = CFG_NUM_AXES;

		r[3] = STATUS_OK;
		break;
	}

	case CMD_CFG_APPLY:
		if (config_validate(&g_cfg))
		{
			apply_all();
			r[1] = STATUS_OK;
		}
		else
		{
			r[1] = STATUS_INVALID;
		}
		break;

	case CMD_CFG_SAVE:
		if (config_save())
		{
			apply_all();
			r[1] = STATUS_OK;
		}
		else
		{
			r[1] = STATUS_INVALID;
		}
		break;

	case CMD_CFG_DEFAULTS:
		config_load_defaults();
		apply_all();
		r[1] = STATUS_OK;
		break;

	case CMD_CFG_LIVE:
	{
		uint8_t const idx = p8[1];
		r[1] = idx;

		if (idx <= 2)
		{
			for (uint8_t i = 0; i < 3; i++)
			{
				uint8_t const axis = (uint8_t)(idx * 3 + i);
				uint16_t const v = (axis < CFG_NUM_AXES) ? panel_get_axis_raw(axis) : 0;
				r[2 + 2 * i] = (uint8_t)(v & 0xFF);
				r[3 + 2 * i] = (uint8_t)(v >> 8);
			}
		}
		else if (idx == 3)
		{
			uint32_t const m = panel_get_input_mask();
			r[2] = (uint8_t)(m);
			r[3] = (uint8_t)(m >> 8);
			r[4] = (uint8_t)(m >> 16);
			r[5] = (uint8_t)(m >> 24);
		}
		break;
	}

	case CMD_CFG_PININFO:
	{
		uint8_t const idx = p8[1];
		r[1] = idx;

		for (uint8_t i = 0; i < 3; i++)
		{
			pin_def_t def;

			if (pins_get((uint8_t)(idx + i), &def))
			{
				r[2 + 2 * i] = def.port;
				r[3 + 2 * i] = (uint8_t)(def.bit | ((def.adc_mux != PIN_NO_ADC) ? 0x80 : 0x00));
			}
			else
			{
				r[2 + 2 * i] = 0xFF;
				r[3 + 2 * i] = 0;
			}
		}
		break;
	}

	default:
		r[1] = STATUS_INVALID;
		break;
	}

	push_reply(r);
}

void proto_handle_report(const uint8_t *p8)
{
	if (proto_is_cfg_cmd(p8[0]))
	{
		handle_cfg(p8);
		return;
	}

	/* SETID / DFU werden vom USB-MCU behandelt (main_usb.c); alles andere ist LED-Wiz.
	 * SETID / DFU are handled by the USB MCU (main_usb.c); everything else is LED-Wiz. */
	if (p8[0] == CMD_SETID || p8[0] == CMD_DFU)
		return;

	outputs_ledwiz_report(p8);
}
