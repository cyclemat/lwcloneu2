/*
 * LWCloneU2 – Arduino Uno (ATmega328P)
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
 * Pin-Index = Arduino-Nummer: 0..13 = D0..D13, 14..19 = A0..A5.
 * D0/D1 sind die UART-Verbindung zum ATmega8U2/16U2 (USB) und daher gesperrt.
 * D0/D1 are the UART link to the ATmega8U2/16U2 (USB) and therefore reserved.
 */

#ifndef PINMAP_H__INCLUDED
#define PINMAP_H__INCLUDED

#define BOARD_ID            BOARD_ARDUINO_UNO
#define WS2812_MAX_PIXELS   64
#define BOARD_HAS_ADC       1

#define BOARD_PIN_TABLE(_m_) \
	_m_( X, 0, PIN_NO_ADC ) /*  0 D0 RX – reserviert / reserved */ \
	_m_( X, 0, PIN_NO_ADC ) /*  1 D1 TX – reserviert / reserved */ \
	_m_( D, 2, PIN_NO_ADC ) /*  2 D2  */ \
	_m_( D, 3, PIN_NO_ADC ) /*  3 D3  */ \
	_m_( D, 4, PIN_NO_ADC ) /*  4 D4  */ \
	_m_( D, 5, PIN_NO_ADC ) /*  5 D5  */ \
	_m_( D, 6, PIN_NO_ADC ) /*  6 D6  */ \
	_m_( D, 7, PIN_NO_ADC ) /*  7 D7  */ \
	_m_( B, 0, PIN_NO_ADC ) /*  8 D8  */ \
	_m_( B, 1, PIN_NO_ADC ) /*  9 D9  */ \
	_m_( B, 2, PIN_NO_ADC ) /* 10 D10 */ \
	_m_( B, 3, PIN_NO_ADC ) /* 11 D11 */ \
	_m_( B, 4, PIN_NO_ADC ) /* 12 D12 */ \
	_m_( B, 5, PIN_NO_ADC ) /* 13 D13 LED */ \
	_m_( C, 0, 0x00       ) /* 14 A0  */ \
	_m_( C, 1, 0x01       ) /* 15 A1  */ \
	_m_( C, 2, 0x02       ) /* 16 A2  */ \
	_m_( C, 3, 0x03       ) /* 17 A3  */ \
	_m_( C, 4, 0x04       ) /* 18 A4  */ \
	_m_( C, 5, 0x05       ) /* 19 A5  */ \
	/* end */

/* Werkseinstellung: Ausgang 1..18 = D2..D13, A0..A5 */
#define BOARD_DEFAULT_OUTPUTS \
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19

#endif
