/*
 * LWCloneU2 – SparkFun Pro Micro / Arduino Micro (ATmega32U4)
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

#ifndef PINMAP_H__INCLUDED
#define PINMAP_H__INCLUDED

#include "../pinmap_32u4.h"

#define BOARD_ID            BOARD_ARDUINO_PROMICRO
#define WS2812_MAX_PIXELS   64

/* Werkseinstellung: Ausgang 1..16 = D2..D10, D14, D15, D16, A0..A3 */
#define BOARD_DEFAULT_OUTPUTS \
	2, 3, 4, 5, 6, 7, 8, 9, 10, 14, 15, 16, 18, 19, 20, 21

#endif
