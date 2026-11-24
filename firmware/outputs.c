/*
 * LWCloneU2 – Ausgänge (LED-Wiz-Logik, Soft-PWM auf GPIO) / outputs
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
 * LED-Wiz-Protokoll (8-Byte-Reports vom Host):
 *   Byte 0 == 64  : SBA – 4 Bank-Bytes (Ein/Aus je Ausgang) + globale Pulsgeschwindigkeit
 *   sonst         : PBA – 8 Profilwerte, vier Reports hintereinander = 32 Ausgänge
 *                   0..48 Helligkeit, 129 Dreieck, 130 Rechteck, 131 Abfall, 132 Anstieg
 *
 * Jeder der 32 Ausgänge ist ein GPIO-Pin mit Soft-PWM (49 Stufen, ~100 Hz) – genau wie beim
 * Original-LED-Wiz. Welcher Pin, steht in g_cfg.out[] (config.h). Der WS2811/WS2812-Strip ist
 * ein eigenes Modul (strip.c) und greift von außen nur über outputs_get_level() auf die
 * aktuellen Helligkeiten zu.
 *
 * Each of the 32 outputs is a GPIO pin with soft PWM (49 steps, ~100 Hz) – just like the original
 * LED-Wiz. Which pin is stored in g_cfg.out[] (config.h). The WS2811/WS2812 strip is a separate
 * module (strip.c) that only reads the current levels via outputs_get_level().
 */

#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include <hwconfig.h>
#include "config.h"
#include "pins.h"
#include "outputs.h"


#if !defined(LED_TIMER_vect)

void outputs_init(void) {}
void outputs_apply_config(void) {}
void outputs_ledwiz_report(const uint8_t *p8bytes) { (void)p8bytes; }
uint8_t outputs_get_level(uint8_t index) { (void)index; return 0; }

#else


#define MAX_PWM             49


/* LED-Wiz-Zustand je Ausgang / LED-Wiz state per output */
static struct {
	volatile uint8_t enable;
	volatile uint8_t mode;
} s_led[CFG_NUM_OUTPUTS];

static volatile uint16_t s_dt = 256;                /* Pulsgeschwindigkeit / pulse speed       */
static volatile uint8_t  s_pwm[CFG_NUM_OUTPUTS];    /* aktuelle Stufe 0..49 / current level    */

/* GPIO-Ausgänge (aufgelöst) / resolved GPIO outputs */
typedef struct {
	pin_io_t io;
	uint8_t idx;        /* LED-Wiz-Ausgang 0..31 */
	uint8_t inv;        /* 0 oder 0xFF */
} gpio_out_t;

static gpio_out_t      s_gpio[CFG_NUM_OUTPUTS];
static volatile uint8_t s_ngpio = 0;      /* vom ISR benutzt / used by the ISR */
static uint8_t         s_ngpio_cfg = 0;   /* belegte Einträge (zum Freigeben) / used entries (for releasing) */


void outputs_ledwiz_report(const uint8_t *p8bytes)
{
	static uint8_t nbank = 0;

	if (p8bytes[0] == 64)
	{
		/* SBA */
		for (uint8_t k = 0; k < 4; k++)
		{
			uint8_t b = p8bytes[1 + k];

			for (uint8_t i = 0; i < 8; i++)
			{
				s_led[k * 8 + i].enable = b & 0x01;
				b >>= 1;
			}
		}

		uint8_t pulse_speed = p8bytes[5];

		if (pulse_speed > 7)
			pulse_speed = 7;

		if (pulse_speed == 0)
			pulse_speed = 1;

		s_dt = (uint16_t)pulse_speed * 128;
		nbank = 0;
	}
	else
	{
		/* PBA, Bank nbank */
		for (uint8_t i = 0; i < 8; i++)
			s_led[nbank * 8 + i].mode = p8bytes[i];

		nbank = (nbank + 1) & 0x03;
	}
}


static uint8_t level_of(uint8_t enable, uint8_t mode, uint16_t t)
{
	if (enable == 0)
		return 0;

	if (mode <= MAX_PWM)
		return mode;                                    /* konstante Helligkeit / constant */

	if (mode == 129)                                    /* Dreieck / triangle */
	{
		uint16_t x = t >> 8;
		if (x & 0x80)
			x = 255 - x;
		return (uint8_t)((MAX_PWM * x) >> 7);
	}

	if (mode == 130)                                    /* Rechteck / square */
		return (t & 0x8000) ? MAX_PWM : 0;

	if (mode == 131)                                    /* Abfall / ramp down */
	{
		uint16_t x = 255 - (t >> 8);
		return (uint8_t)((MAX_PWM * x) >> 8);
	}

	if (mode == 132)                                    /* Anstieg / ramp up */
	{
		uint16_t x = t >> 8;
		return (uint8_t)((MAX_PWM * x) >> 8);
	}

	return 0;
}


ISR(LED_TIMER_vect)
{
	static int8_t counter = 0;
	static uint16_t t = 0;

	counter--;

	if (counter < 0)
	{
		/* Stufe MAX_PWM ist immer an, 0 immer aus / level MAX_PWM is always on, 0 always off */
		counter = MAX_PWM - 1;
		t += s_dt;

		for (uint8_t i = 0; i < CFG_NUM_OUTPUTS; i++)
			s_pwm[i] = level_of(s_led[i].enable, s_led[i].mode, t);
	}

	/* GPIO-Pins setzen / drive GPIO pins */
	uint8_t const n = s_ngpio;

	for (uint8_t i = 0; i < n; i++)
	{
		gpio_out_t const * const g = &s_gpio[i];
		uint8_t const on = (uint8_t)((s_pwm[g->idx] > (uint8_t)counter) ? 0xFF : 0x00) ^ g->inv;

		if (on)
			*g->io.port |= g->io.mask;
		else
			*g->io.port &= (uint8_t)~g->io.mask;
	}
}


void outputs_apply_config(void)
{
	/* 1. alte Ausgänge abschalten und Pins freigeben / disable old outputs, release pins */
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		s_ngpio = 0;
	}

	for (uint8_t i = 0; i < s_ngpio_cfg; i++)
		pins_mode_input(&s_gpio[i].io, false);

	s_ngpio_cfg = 0;

	/* 2. Ausgänge auflösen / resolve outputs */
	uint8_t ngpio = 0;

	for (uint8_t i = 0; i < CFG_NUM_OUTPUTS; i++)
	{
		cfg_output_t const * const o = &g_cfg.out[i];
		pin_io_t io;

		if (o->pin == CFG_PIN_NONE || !pins_resolve(o->pin, &io))
			continue;

		pins_mode_output(&io);

		s_gpio[ngpio].io   = io;
		s_gpio[ngpio].idx  = i;
		s_gpio[ngpio].inv  = (o->flags & OUTF_INVERT) ? 0xFF : 0x00;
		ngpio++;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		s_ngpio_cfg = ngpio;
		s_ngpio = ngpio;
	}
}


void outputs_init(void)
{
	memset(s_led, 0x00, sizeof(s_led));
	memset((void *)s_pwm, 0x00, sizeof(s_pwm));

	outputs_apply_config();
	led_timer_init();
}


uint8_t outputs_get_level(uint8_t index)
{
	return (index < CFG_NUM_OUTPUTS) ? s_pwm[index] : 0;
}


#endif
