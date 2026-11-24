/*
 * LWCloneU2 (Due) – Pin-Tabelle aus dem Arduino-SAM-Core (g_APinDescription) / pin table from the core
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

#include <Arduino.h>
#include "lwc_shared.h"


/* D0/D1 hängen am ATmega16U2 (Programming-Port) – nicht belegbar / wired to the 16U2 – not assignable */
static bool is_reserved(uint8_t index)
{
	return index == 0 || index == 1;
}

static uint8_t port_letter(const Pio *pio)
{
	if (pio == PIOA) return PA_;
	if (pio == PIOB) return PB_;
	if (pio == PIOC) return PC_;
	if (pio == PIOD) return PD_;
	return PX_;
}

static uint8_t bit_of(uint32_t mask)
{
	return (uint8_t)__builtin_ctz(mask);
}

static uint8_t adc_channel(const PinDescription *d)
{
	/* ulADCChannelNumber: ADC0..ADC15 = 0..15, DA0/DA1 und NO_ADC sind kein ADC / are not ADC */
	int32_t const ch = (int32_t)d->ulADCChannelNumber;
	return (ch >= 0 && ch < 16) ? (uint8_t)ch : PIN_NO_ADC;
}


extern "C" {

uint8_t pins_count(void)
{
	return LWC_NUM_PINS;
}

bool pins_get(uint8_t index, pin_def_t *def)
{
	if (index >= LWC_NUM_PINS || def == NULL)
		return false;

	PinDescription const * const d = &g_APinDescription[index];

	def->port    = is_reserved(index) ? PX_ : port_letter(d->pPort);
	def->bit     = bit_of(d->ulPin);
	def->adc_mux = adc_channel(d);
	return true;
}

bool pins_resolve(uint8_t index, pin_io_t *io)
{
	if (io == NULL || index >= LWC_NUM_PINS || is_reserved(index))
		return false;

	PinDescription const * const d = &g_APinDescription[index];

	if (d->pPort == NULL || d->ulPin == 0)
		return false;

	io->reg  = &d->pPort->PIO_SODR;   /* CODR/ODSR/PDSR folgen (pins.h) / follow (pins.h) */
	io->mask = d->ulPin;
	return true;
}

uint8_t pins_adc_mux(uint8_t index)
{
	pin_def_t def;
	return pins_get(index, &def) ? def.adc_mux : PIN_NO_ADC;
}

bool pins_has_adc(uint8_t index)
{
	return pins_adc_mux(index) != PIN_NO_ADC;
}

/* Pio-Basis aus dem SODR-Zeiger zurückrechnen / recover the Pio base from the SODR pointer */
static Pio *pio_of(const pin_io_t *io)
{
	return (Pio *)((uintptr_t)io->reg - offsetof(Pio, PIO_SODR));
}

void pins_mode_input(const pin_io_t *io, bool pullup)
{
	Pio * const pio = pio_of(io);
	pmc_enable_periph_clk(port_letter(pio) == PA_ ? ID_PIOA : port_letter(pio) == PB_ ? ID_PIOB :
	                      port_letter(pio) == PC_ ? ID_PIOC : ID_PIOD);   /* Eingänge brauchen den Takt / inputs need the clock */
	PIO_Configure(pio, PIO_INPUT, io->mask, pullup ? PIO_PULLUP : PIO_DEFAULT);
}

void pins_mode_output(const pin_io_t *io)
{
	PIO_Configure(pio_of(io), PIO_OUTPUT_0, io->mask, PIO_DEFAULT);   /* Ausgang, low, kein Pull-up / output, low, no pull-up */
}

} /* extern "C" */
