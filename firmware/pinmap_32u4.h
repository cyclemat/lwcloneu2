/*
 * LWCloneU2 – Pin-Tabelle ATmega32U4 (Arduino Leonardo / Micro / Pro Micro)
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
 * Pin-Index = Arduino-Nummer. Das Konfigurations-Tool zeigt die Namen an.
 * Pin index = Arduino number. The configuration tool shows the names.
 *
 *   0..13  D0..D13        14 D14/MISO   15 D15/SCK   16 D16/MOSI   17 D17/RXLED
 *  18..23  A0..A5         24 TXLED
 *
 * Pro Micro: D11, D12, D13, A4, A5 sind nicht herausgeführt / not available on the Pro Micro.
 */

#ifndef PINMAP_32U4_H__INCLUDED
#define PINMAP_32U4_H__INCLUDED

#define BOARD_PIN_TABLE(_m_) \
	_m_( D, 2, PIN_NO_ADC ) /*  0  D0  RX1  */ \
	_m_( D, 3, PIN_NO_ADC ) /*  1  D1  TX1  */ \
	_m_( D, 1, PIN_NO_ADC ) /*  2  D2  SDA  */ \
	_m_( D, 0, PIN_NO_ADC ) /*  3  D3  SCL  */ \
	_m_( D, 4, 0x20       ) /*  4  D4  A6   */ \
	_m_( C, 6, PIN_NO_ADC ) /*  5  D5       */ \
	_m_( D, 7, 0x22       ) /*  6  D6  A7   */ \
	_m_( E, 6, PIN_NO_ADC ) /*  7  D7       */ \
	_m_( B, 4, 0x23       ) /*  8  D8  A8   */ \
	_m_( B, 5, 0x24       ) /*  9  D9  A9   */ \
	_m_( B, 6, 0x25       ) /* 10  D10 A10  */ \
	_m_( B, 7, PIN_NO_ADC ) /* 11  D11      */ \
	_m_( D, 6, 0x21       ) /* 12  D12 A11  */ \
	_m_( C, 7, PIN_NO_ADC ) /* 13  D13 LED  */ \
	_m_( B, 3, PIN_NO_ADC ) /* 14  D14 MISO */ \
	_m_( B, 1, PIN_NO_ADC ) /* 15  D15 SCK  */ \
	_m_( B, 2, PIN_NO_ADC ) /* 16  D16 MOSI */ \
	_m_( B, 0, PIN_NO_ADC ) /* 17  D17 RXLED (aktiv low) */ \
	_m_( F, 7, 0x07       ) /* 18  A0       */ \
	_m_( F, 6, 0x06       ) /* 19  A1       */ \
	_m_( F, 5, 0x05       ) /* 20  A2       */ \
	_m_( F, 4, 0x04       ) /* 21  A3       */ \
	_m_( F, 1, 0x01       ) /* 22  A4       */ \
	_m_( F, 0, 0x00       ) /* 23  A5       */ \
	_m_( D, 5, PIN_NO_ADC ) /* 24  TXLED (aktiv low) */ \
	/* end */

#define BOARD_HAS_ADC 1

#endif
