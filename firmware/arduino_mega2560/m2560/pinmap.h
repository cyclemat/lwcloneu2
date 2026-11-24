/*
 * LWCloneU2 – Arduino Mega 2560 (ATmega2560)
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
 * Pin-Index = Arduino-Nummer: 0..53 = D0..D53, 54..69 = A0..A15.
 * D0/D1 sind die UART0-Verbindung zum ATmega16U2 (USB) und daher gesperrt.
 * D0/D1 are the UART0 link to the ATmega16U2 (USB) and therefore reserved.
 */

#ifndef PINMAP_H__INCLUDED
#define PINMAP_H__INCLUDED

#define BOARD_ID            BOARD_ARDUINO_MEGA2560
#define WS2812_MAX_PIXELS   128
#define BOARD_HAS_ADC       1

#define BOARD_PIN_TABLE(_m_) \
	_m_( X, 0, PIN_NO_ADC ) /*  0 D0  RX0 – reserviert / reserved */ \
	_m_( X, 0, PIN_NO_ADC ) /*  1 D1  TX0 – reserviert / reserved */ \
	_m_( E, 4, PIN_NO_ADC ) /*  2 D2  */ \
	_m_( E, 5, PIN_NO_ADC ) /*  3 D3  */ \
	_m_( G, 5, PIN_NO_ADC ) /*  4 D4  */ \
	_m_( E, 3, PIN_NO_ADC ) /*  5 D5  */ \
	_m_( H, 3, PIN_NO_ADC ) /*  6 D6  */ \
	_m_( H, 4, PIN_NO_ADC ) /*  7 D7  */ \
	_m_( H, 5, PIN_NO_ADC ) /*  8 D8  */ \
	_m_( H, 6, PIN_NO_ADC ) /*  9 D9  */ \
	_m_( B, 4, PIN_NO_ADC ) /* 10 D10 */ \
	_m_( B, 5, PIN_NO_ADC ) /* 11 D11 */ \
	_m_( B, 6, PIN_NO_ADC ) /* 12 D12 */ \
	_m_( B, 7, PIN_NO_ADC ) /* 13 D13 LED */ \
	_m_( J, 1, PIN_NO_ADC ) /* 14 D14 TX3 */ \
	_m_( J, 0, PIN_NO_ADC ) /* 15 D15 RX3 */ \
	_m_( H, 1, PIN_NO_ADC ) /* 16 D16 TX2 */ \
	_m_( H, 0, PIN_NO_ADC ) /* 17 D17 RX2 */ \
	_m_( D, 3, PIN_NO_ADC ) /* 18 D18 TX1 */ \
	_m_( D, 2, PIN_NO_ADC ) /* 19 D19 RX1 */ \
	_m_( D, 1, PIN_NO_ADC ) /* 20 D20 SDA */ \
	_m_( D, 0, PIN_NO_ADC ) /* 21 D21 SCL */ \
	_m_( A, 0, PIN_NO_ADC ) /* 22 D22 */ \
	_m_( A, 1, PIN_NO_ADC ) /* 23 D23 */ \
	_m_( A, 2, PIN_NO_ADC ) /* 24 D24 */ \
	_m_( A, 3, PIN_NO_ADC ) /* 25 D25 */ \
	_m_( A, 4, PIN_NO_ADC ) /* 26 D26 */ \
	_m_( A, 5, PIN_NO_ADC ) /* 27 D27 */ \
	_m_( A, 6, PIN_NO_ADC ) /* 28 D28 */ \
	_m_( A, 7, PIN_NO_ADC ) /* 29 D29 */ \
	_m_( C, 7, PIN_NO_ADC ) /* 30 D30 */ \
	_m_( C, 6, PIN_NO_ADC ) /* 31 D31 */ \
	_m_( C, 5, PIN_NO_ADC ) /* 32 D32 */ \
	_m_( C, 4, PIN_NO_ADC ) /* 33 D33 */ \
	_m_( C, 3, PIN_NO_ADC ) /* 34 D34 */ \
	_m_( C, 2, PIN_NO_ADC ) /* 35 D35 */ \
	_m_( C, 1, PIN_NO_ADC ) /* 36 D36 */ \
	_m_( C, 0, PIN_NO_ADC ) /* 37 D37 */ \
	_m_( D, 7, PIN_NO_ADC ) /* 38 D38 */ \
	_m_( G, 2, PIN_NO_ADC ) /* 39 D39 */ \
	_m_( G, 1, PIN_NO_ADC ) /* 40 D40 */ \
	_m_( G, 0, PIN_NO_ADC ) /* 41 D41 */ \
	_m_( L, 7, PIN_NO_ADC ) /* 42 D42 */ \
	_m_( L, 6, PIN_NO_ADC ) /* 43 D43 */ \
	_m_( L, 5, PIN_NO_ADC ) /* 44 D44 */ \
	_m_( L, 4, PIN_NO_ADC ) /* 45 D45 */ \
	_m_( L, 3, PIN_NO_ADC ) /* 46 D46 */ \
	_m_( L, 2, PIN_NO_ADC ) /* 47 D47 */ \
	_m_( L, 1, PIN_NO_ADC ) /* 48 D48 */ \
	_m_( L, 0, PIN_NO_ADC ) /* 49 D49 */ \
	_m_( B, 3, PIN_NO_ADC ) /* 50 D50 MISO */ \
	_m_( B, 2, PIN_NO_ADC ) /* 51 D51 MOSI */ \
	_m_( B, 1, PIN_NO_ADC ) /* 52 D52 SCK  */ \
	_m_( B, 0, PIN_NO_ADC ) /* 53 D53 SS   */ \
	_m_( F, 0, 0x00       ) /* 54 A0  */ \
	_m_( F, 1, 0x01       ) /* 55 A1  */ \
	_m_( F, 2, 0x02       ) /* 56 A2  */ \
	_m_( F, 3, 0x03       ) /* 57 A3  */ \
	_m_( F, 4, 0x04       ) /* 58 A4  */ \
	_m_( F, 5, 0x05       ) /* 59 A5  */ \
	_m_( F, 6, 0x06       ) /* 60 A6  */ \
	_m_( F, 7, 0x07       ) /* 61 A7  */ \
	_m_( K, 0, 0x20       ) /* 62 A8  */ \
	_m_( K, 1, 0x21       ) /* 63 A9  */ \
	_m_( K, 2, 0x22       ) /* 64 A10 */ \
	_m_( K, 3, 0x23       ) /* 65 A11 */ \
	_m_( K, 4, 0x24       ) /* 66 A12 */ \
	_m_( K, 5, 0x25       ) /* 67 A13 */ \
	_m_( K, 6, 0x26       ) /* 68 A14 */ \
	_m_( K, 7, 0x27       ) /* 69 A15 */ \
	/* end */

/* Werkseinstellung: Ausgang 1..32 = D22..D53 (hintere Stiftleiste) / outputs 1..32 = D22..D53 */
#define BOARD_DEFAULT_OUTPUTS \
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, \
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53

#endif
