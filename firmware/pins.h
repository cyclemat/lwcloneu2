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

/*
 * Jedes Board liefert in seiner pinmap.h eine Tabelle BOARD_PIN_TABLE(_m_) mit Einträgen
 *     _m_( PORT, BIT, ADC_MUX )
 * in der Reihenfolge der Arduino-Pin-Nummern (D0, D1, ... dann A0, A1, ...). Der Index in
 * dieser Tabelle ist der "Pin-Index", den die Konfiguration (config.h) benutzt. ADC_MUX ist
 * der Wert für ADMUX (plus MUX5 in Bit 5) oder PIN_NO_ADC. PORT = X markiert einen gesperrten Pin.
 *
 * Each board provides a table BOARD_PIN_TABLE(_m_) in its pinmap.h with entries
 *     _m_( PORT, BIT, ADC_MUX )
 * ordered like the Arduino pin numbers (D0, D1, ... then A0, A1, ...). The index into this
 * table is the "pin index" used by the configuration (config.h). ADC_MUX is the ADMUX value
 * (plus MUX5 in bit 5) or PIN_NO_ADC. PORT = X marks a reserved pin.
 */

#ifndef PINS_H__INCLUDED
#define PINS_H__INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#define PIN_NO_ADC   0xFF

/* Board-Kennungen (werden per USB gemeldet) / board ids (reported via USB) */
enum {
	BOARD_UNKNOWN          = 0,
	BOARD_ARDUINO_LEONARDO = 1,  /* ATmega32U4, 16 MHz */
	BOARD_ARDUINO_PROMICRO = 2,  /* ATmega32U4, 16 MHz, weniger Pins / fewer pins */
	BOARD_ARDUINO_MEGA2560 = 3,  /* ATmega2560 hinter ATmega16U2 / behind ATmega16U2 */
	BOARD_ARDUINO_UNO      = 4,  /* ATmega328P hinter ATmega8U2/16U2 / behind ATmega8U2/16U2 */
	BOARD_BREAKOUT_32U2    = 5,  /* eigenes Breakout mit ATmega32U2 / custom breakout */
};

/* Port-Buchstaben als Index / port letters as index */
enum { PA_ = 0, PB_, PC_, PD_, PE_, PF_, PG_, PH_, PJ_, PK_, PL_, NUM_PORT_LETTERS,
       PX_ = 0xFE   /* reservierter Pin (z. B. UART zum USB-Chip), nicht belegbar / reserved pin, not assignable */ };

typedef struct {
	uint8_t port;     /* PA_ .. PL_ oder PX_ */
	uint8_t bit;      /* 0..7 */
	uint8_t adc_mux;  /* ADMUX-Wert oder PIN_NO_ADC */
} pin_def_t;

/* Aufgelöster Pin für schnellen Zugriff im Betrieb. Bei allen AVR8 liegen PINx, DDRx, PORTx
 * direkt hintereinander, daher reicht die PORTx-Adresse.
 * Resolved pin for fast runtime access. On all AVR8 PINx, DDRx, PORTx are consecutive,
 * so the PORTx address is enough. */
typedef struct {
	volatile uint8_t *port;  /* PORTx */
	uint8_t mask;
} pin_io_t;

#define PIN_REG_PORT(io)  ((io)->port)
#define PIN_REG_DDR(io)   ((io)->port - 1)
#define PIN_REG_PIN(io)   ((io)->port - 2)

uint8_t pins_count(void);
bool    pins_get(uint8_t index, pin_def_t *def);            /* false wenn Index ungültig / if index invalid */
bool    pins_resolve(uint8_t index, pin_io_t *io);          /* false wenn ungültig oder gesperrt / if invalid or reserved */
bool    pins_has_adc(uint8_t index);
uint8_t pins_adc_mux(uint8_t index);                         /* PIN_NO_ADC wenn nicht ADC-fähig */

/* Eingang mit/ohne Pull-up, Ausgang low / input with(out) pull-up, output low */
void pins_mode_input(const pin_io_t *io, bool pullup);
void pins_mode_output(const pin_io_t *io);

#endif
