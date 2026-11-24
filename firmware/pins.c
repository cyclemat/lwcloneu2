/*
 * LWCloneU2 – Pin-Tabellen der Boards / board pin tables
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

#include <stddef.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

#include <hwconfig.h>
#include "pins.h"


#if !defined(BOARD_PIN_TABLE)
	#error "pinmap.h des Boards muss BOARD_PIN_TABLE(_m_) definieren / board pinmap.h must define BOARD_PIN_TABLE(_m_)"
#endif

#define MAP(port, bit, mux) { P##port##_, bit, mux },
static const pin_def_t PROGMEM s_pins[] = { BOARD_PIN_TABLE(MAP) };
#undef MAP

#define NUM_PINS (sizeof(s_pins) / sizeof(s_pins[0]))

_Static_assert(NUM_PINS <= 254, "zu viele Pins (max. 254) / too many pins");


/* PORTx-Adressen je Port-Buchstabe; nicht vorhandene Ports sind 0.
 * PORTx addresses per port letter; ports the MCU lacks are 0. */
#define PORT_ENTRY(X)  (uint16_t)(uintptr_t)&PORT##X,
#define PORT_MISSING   0,

static const uint16_t PROGMEM s_ports[NUM_PORT_LETTERS] = {
	#if defined(PORTA)
	PORT_ENTRY(A)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTB)
	PORT_ENTRY(B)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTC)
	PORT_ENTRY(C)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTD)
	PORT_ENTRY(D)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTE)
	PORT_ENTRY(E)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTF)
	PORT_ENTRY(F)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTG)
	PORT_ENTRY(G)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTH)
	PORT_ENTRY(H)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTJ)
	PORT_ENTRY(J)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTK)
	PORT_ENTRY(K)
	#else
	PORT_MISSING
	#endif
	#if defined(PORTL)
	PORT_ENTRY(L)
	#else
	PORT_MISSING
	#endif
};


uint8_t pins_count(void)
{
	return (uint8_t)NUM_PINS;
}

bool pins_get(uint8_t index, pin_def_t *def)
{
	if (index >= NUM_PINS || def == NULL)
		return false;

	memcpy_P(def, &s_pins[index], sizeof(*def));
	return true;
}

bool pins_resolve(uint8_t index, pin_io_t *io)
{
	pin_def_t def;

	if (io == NULL || !pins_get(index, &def) || def.port >= NUM_PORT_LETTERS || def.bit > 7)
		return false;

	uint16_t const addr = pgm_read_word(&s_ports[def.port]);

	if (addr == 0)
		return false;

	io->port = (volatile uint8_t *)(uintptr_t)addr;
	io->mask = (uint8_t)(1 << def.bit);
	return true;
}

uint8_t pins_adc_mux(uint8_t index)
{
	pin_def_t def;

	if (!pins_get(index, &def))
		return PIN_NO_ADC;

	return def.adc_mux;
}

bool pins_has_adc(uint8_t index)
{
	return pins_adc_mux(index) != PIN_NO_ADC;
}

void pins_mode_input(const pin_io_t *io, bool pullup)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		*PIN_REG_DDR(io) &= (uint8_t)~io->mask;
		if (pullup)
			*PIN_REG_PORT(io) |= io->mask;
		else
			*PIN_REG_PORT(io) &= (uint8_t)~io->mask;
	}
}

void pins_mode_output(const pin_io_t *io)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		*PIN_REG_PORT(io) &= (uint8_t)~io->mask;
		*PIN_REG_DDR(io)  |= io->mask;
	}
}
