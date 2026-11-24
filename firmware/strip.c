/*
 * LWCloneU2 – WS2811/WS2812-Strip (eigenes Modul) / LED strip (separate module)
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
#include <stdint.h>
#include <avr/io.h>
#include <util/atomic.h>

#include <hwconfig.h>
#include "config.h"
#include "pins.h"
#include "outputs.h"
#include "ws2812.h"
#include "clock.h"
#include "proto.h"
#include "strip.h"
#include "strip_serial.h"


#if (WS2812_MAX_PIXELS == 0)

/* Board ohne Strip: alles leer / board without strip: stubs */
void strip_init(void) {}
void strip_apply_config(void) {}
void strip_task(void) {}
bool strip_handle_report(const uint8_t *p8, uint8_t *reply8)
{
	memset(reply8, 0x00, PROTO_REPORT_SIZE);
	reply8[0] = p8[0];
	reply8[1] = STATUS_INVALID;
	return true;
}
bool    strip_direct_ready(void) { return false; }
uint8_t strip_count(void) { return 0; }
bool    strip_set_count(uint8_t n) { (void)n; return false; }
void    strip_set_pixel(uint8_t idx, uint8_t r, uint8_t g, uint8_t b) { (void)idx; (void)r; (void)g; (void)b; }
void    strip_fill(uint8_t first, uint8_t n, uint8_t r, uint8_t g, uint8_t b) { (void)first; (void)n; (void)r; (void)g; (void)b; }
void    strip_clear(void) {}
void    strip_show(uint8_t source) { (void)source; }

#else


static pin_io_t  s_io;
static uint8_t   s_active = 0;
static uint8_t   s_count  = 0;
static uint8_t   s_mode   = STRIP_MODE_DIRECT;
static uint8_t   s_pos[3];                        /* Byteposition von R, G, B im Pixel / byte position of R, G, B */
static uint8_t   s_pixels[WS2812_MAX_PIXELS * 3]; /* in Strip-Reihenfolge / in strip order */
static uint8_t   s_dirty  = 0;                    /* Ausgabe angefordert / output requested */
static uint8_t   s_show_pending = 0;              /* Bitmaske STRIP_SHOW_*: wer wartet auf Antwort / who awaits a reply */
static uint8_t   s_last_level[CFG_NUM_OUTPUTS];   /* Spiegel: zuletzt gesehene Stufen / mirror: last levels */


static void set_order(uint8_t order)
{
	switch (order)
	{
	case WS_ORDER_RGB: s_pos[0] = 0; s_pos[1] = 1; s_pos[2] = 2; break;
	case WS_ORDER_BRG: s_pos[0] = 1; s_pos[1] = 2; s_pos[2] = 0; break;
	default:           s_pos[0] = 1; s_pos[1] = 0; s_pos[2] = 2; break;   /* GRB */
	}
}

static inline void put_pixel(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t * const px = &s_pixels[idx * 3];
	px[s_pos[0]] = r;
	px[s_pos[1]] = g;
	px[s_pos[2]] = b;
}

/* je Kanal gewinnt der hellere Wert / brighter value wins per channel */
static inline void max_pixel(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t * const px = &s_pixels[idx * 3];
	if (px[s_pos[0]] < r) px[s_pos[0]] = r;
	if (px[s_pos[1]] < g) px[s_pos[1]] = g;
	if (px[s_pos[2]] < b) px[s_pos[2]] = b;
}


void strip_apply_config(void)
{
	if (s_active)
	{
		/* Strip dunkel schalten und Pin freigeben / blank the strip, release the pin */
		memset(s_pixels, 0x00, (uint16_t)s_count * 3);
		ws2812_send(&s_io, s_pixels, (uint16_t)s_count * 3);
		pins_mode_input(&s_io, false);
		s_active = 0;
	}

	s_count = 0;
	s_dirty = 0;
	s_show_pending = 0;
	memset(s_last_level, 0xFF, sizeof(s_last_level));   /* erzwingt erstes Rendern / forces first render */

	if (g_cfg.ws.pin == CFG_PIN_NONE || g_cfg.ws.count == 0 || g_cfg.ws.count > WS2812_MAX_PIXELS)
		return;

	if (!pins_resolve(g_cfg.ws.pin, &s_io))
		return;

	pins_mode_output(&s_io);
	set_order(g_cfg.ws.order);

	s_count  = g_cfg.ws.count;
	s_mode   = g_cfg.ws.mode;
	s_active = 1;

	memset(s_pixels, 0x00, sizeof(s_pixels));
	s_dirty = 1;    /* einmal dunkel schreiben / write black once */
}


void strip_init(void)
{
	strip_apply_config();
}


/* Stufe 0..OUTPUTS_MAX_LEVEL → 0..255 */
static uint8_t level_to_8bit(uint8_t level)
{
	if (level >= OUTPUTS_MAX_LEVEL)
		return 255;

	return (uint8_t)(((uint16_t)level * 255 + (OUTPUTS_MAX_LEVEL / 2)) / OUTPUTS_MAX_LEVEL);
}

static inline uint8_t scale(uint8_t c, uint8_t v)
{
	return (uint8_t)(((uint16_t)c * v + 127) / 255);
}

/* Spiegel-Modus: Pixel aus den Ausgangsstufen berechnen; true wenn sich etwas geändert hat
 * mirror mode: compute pixels from output levels; true if anything changed */
static bool mirror_render(void)
{
	uint8_t level[CFG_NUM_OUTPUTS];
	bool changed = false;

	for (uint8_t i = 0; i < CFG_NUM_OUTPUTS; i++)
	{
		level[i] = outputs_get_level(i);

		if (level[i] != s_last_level[i])
		{
			s_last_level[i] = level[i];
			changed = true;
		}
	}

	if (!changed)
		return false;

	memset(s_pixels, 0x00, (uint16_t)s_count * 3);

	for (uint8_t k = 0; k < CFG_NUM_MIRROR; k++)
	{
		cfg_mirror_t const * const m = &g_cfg.mirror[k];

		if (m->out >= CFG_NUM_OUTPUTS || m->count == 0)
			continue;

		uint8_t const v = level_to_8bit(level[m->out]);

		if (v == 0)
			continue;

		uint8_t const r = scale(m->r, v), g = scale(m->g, v), b = scale(m->b, v);
		uint16_t const last = (uint16_t)m->first + m->count;

		for (uint16_t p = m->first; p < last && p < s_count; p++)
			max_pixel((uint8_t)p, r, g, b);
	}

	return true;
}


void strip_task(void)
{
	static uint16_t t_last = 0;

	if (!s_active)
		return;

	uint16_t const now = clock_ms();

	if ((uint16_t)(now - t_last) < STRIP_REFRESH_MS)
		return;

	if (s_mode == STRIP_MODE_MIRROR && mirror_render())
		s_dirty = 1;

	if (!s_dirty)
		return;

	t_last  = now;
	s_dirty = 0;

	ws2812_send(&s_io, s_pixels, (uint16_t)s_count * 3);

	/* Antworten auf SHOW erst jetzt – der Host weiß damit, dass der Strip geschrieben ist
	 * replies to SHOW only now – tells the host the strip has been written */
	uint8_t const pending = s_show_pending;
	s_show_pending = 0;

	if (pending & STRIP_SHOW_HID)
	{
		uint8_t r[PROTO_REPORT_SIZE];
		memset(r, 0x00, sizeof(r));
		r[0] = CMD_STRIP_SHOW;
		r[1] = STATUS_OK;
		proto_push_reply(r);
	}

	if (pending & STRIP_SHOW_SERIAL)
		strip_serial_show_done();
}


/* ------------------------------------------------------------------ Direkt-Zugriff / direct access */

bool strip_direct_ready(void)
{
	return s_active && s_mode == STRIP_MODE_DIRECT;
}

uint8_t strip_count(void)
{
	return s_active ? s_count : 0;
}

bool strip_set_count(uint8_t n)
{
	if (!s_active || n == 0 || n > WS2812_MAX_PIXELS)
		return false;

	if (n != s_count)
	{
		/* Pixel hinter der neuen Länge sofort dunkel, damit nichts stehen bleibt
		 * pixels beyond the new length go dark right away so nothing stays lit */
		if (n < s_count)
			ws2812_send(&s_io, s_pixels, (uint16_t)s_count * 3);  /* letzter Stand / last state */
		memset(s_pixels, 0x00, sizeof(s_pixels));
		s_count = n;
		s_dirty = 1;
	}
	return true;
}

void strip_set_pixel(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
	if (idx < s_count)
		put_pixel(idx, r, g, b);
}

void strip_fill(uint8_t first, uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t last = (uint16_t)first + n;

	if (last > s_count)
		last = s_count;

	for (uint16_t p = first; p < last; p++)
		put_pixel((uint8_t)p, r, g, b);
}

void strip_clear(void)
{
	memset(s_pixels, 0x00, sizeof(s_pixels));
}

void strip_show(uint8_t source)
{
	s_dirty = 1;
	s_show_pending |= source;
}


bool strip_handle_report(const uint8_t *p8, uint8_t *reply8)
{
	memset(reply8, 0x00, PROTO_REPORT_SIZE);
	reply8[0] = p8[0];

	switch (p8[0])
	{
	case CMD_STRIP_INFO:
		reply8[1] = s_active ? s_count : 0;
		reply8[2] = WS2812_MAX_PIXELS;
		reply8[3] = s_mode;
		reply8[4] = g_cfg.ws.order;
		return true;

	case CMD_STRIP_SET:
	{
		/* [B1, idx, r, g, b, r, g, b] – 2 Pixel ab idx / 2 pixels from idx */
		if (!s_active || s_mode != STRIP_MODE_DIRECT)
			return false;

		uint8_t const idx = p8[1];

		if (idx < s_count)
			put_pixel(idx, p8[2], p8[3], p8[4]);

		if ((uint8_t)(idx + 1) < s_count && idx != 0xFF)
			put_pixel((uint8_t)(idx + 1), p8[5], p8[6], p8[7]);

		return false;
	}

	case CMD_STRIP_FILL:
	{
		/* [B2, idx, n, r, g, b] – n Pixel ab idx, n == 0: bis zum Ende / n pixels from idx, 0 = to the end */
		if (!s_active || s_mode != STRIP_MODE_DIRECT)
			return false;

		uint16_t const first = p8[1];
		uint16_t last = (p8[2] == 0) ? s_count : first + p8[2];

		if (last > s_count)
			last = s_count;

		for (uint16_t p = first; p < last; p++)
			put_pixel((uint8_t)p, p8[3], p8[4], p8[5]);

		return false;
	}

	case CMD_STRIP_SHOW:
		if (!s_active || s_mode != STRIP_MODE_DIRECT)
		{
			reply8[1] = STATUS_INVALID;
			return true;
		}

		strip_show(STRIP_SHOW_HID);
		return false;       /* Antwort kommt aus strip_task() / reply comes from strip_task() */

	default:
		reply8[1] = STATUS_INVALID;
		return true;
	}
}


#endif
