/*
 * LWCloneU2 – Breakout-Board ATmega32U2 (schematic/breakout32u2.sch)
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
 * Pin-Index: 0..7 = PB0..PB7, 8 = PC2, 9 = PC4, 10 = PC5, 11 = PC6, 12 = PC7, 13..20 = PD0..PD7.
 * Der ATmega32U2 hat keinen ADC – keine Analog-Achsen. / No ADC on the ATmega32U2 – no analog axes.
 */

#ifndef PINMAP_H__INCLUDED
#define PINMAP_H__INCLUDED

#define BOARD_ID            BOARD_BREAKOUT_32U2

/* nur 1 KB RAM: kleinere Tabellen / only 1 KB RAM: smaller tables */
#define WS2812_MAX_PIXELS   8
#define CFG_NUM_INPUTS      16
#define CFG_NUM_AXES        0     /* kein ADC / no ADC */
#define CFG_NUM_MIRROR      8     /* wenig RAM / little RAM */

#define BOARD_PIN_TABLE(_m_) \
	_m_( B, 0, PIN_NO_ADC ) /*  0 PB0 */ \
	_m_( B, 1, PIN_NO_ADC ) /*  1 PB1 */ \
	_m_( B, 2, PIN_NO_ADC ) /*  2 PB2 */ \
	_m_( B, 3, PIN_NO_ADC ) /*  3 PB3 */ \
	_m_( B, 4, PIN_NO_ADC ) /*  4 PB4 */ \
	_m_( B, 5, PIN_NO_ADC ) /*  5 PB5 */ \
	_m_( B, 6, PIN_NO_ADC ) /*  6 PB6 */ \
	_m_( B, 7, PIN_NO_ADC ) /*  7 PB7 */ \
	_m_( C, 2, PIN_NO_ADC ) /*  8 PC2 */ \
	_m_( C, 4, PIN_NO_ADC ) /*  9 PC4 */ \
	_m_( C, 5, PIN_NO_ADC ) /* 10 PC5 */ \
	_m_( C, 6, PIN_NO_ADC ) /* 11 PC6 */ \
	_m_( C, 7, PIN_NO_ADC ) /* 12 PC7 */ \
	_m_( D, 0, PIN_NO_ADC ) /* 13 PD0 */ \
	_m_( D, 1, PIN_NO_ADC ) /* 14 PD1 */ \
	_m_( D, 2, PIN_NO_ADC ) /* 15 PD2 RX1 */ \
	_m_( D, 3, PIN_NO_ADC ) /* 16 PD3 TX1 */ \
	_m_( D, 4, PIN_NO_ADC ) /* 17 PD4 */ \
	_m_( D, 5, PIN_NO_ADC ) /* 18 PD5 */ \
	_m_( D, 6, PIN_NO_ADC ) /* 19 PD6 */ \
	_m_( D, 7, PIN_NO_ADC ) /* 20 PD7 */ \
	/* end */

/* Werkseinstellung wie die bisherige feste Belegung / factory default like the previous fixed mapping:
 * Ausgang 1..6 = PB3, PB2, PB1, PD6, PD7, PB0 */
#define BOARD_DEFAULT_OUTPUTS \
	3, 2, 1, 19, 20, 0

/* Eingänge (Pin, Funktion) / inputs (pin, function) */
#define BOARD_DEFAULT_INPUTS \
	13, KEY_1,        /* PD0 */ \
	14, J1_Button1,   /* PD1 */ \
	15, KEY_2,        /* PD2 */ \
	6,  KEY_5,        /* PB6 */ \
	7,  KEY_5,        /* PB7 */ \
	10, J1_Button2,   /* PC5 */ \
	9,  J1_Right,     /* PC4 */ \
	12, J1_Left,      /* PC7 */ \
	8,  J1_Down,      /* PC2 */ \
	5,  J1_Up         /* PB5 */

#endif
