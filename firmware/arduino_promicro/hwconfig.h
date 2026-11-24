/*
 * LWCloneU2 – SparkFun Pro Micro / Arduino Micro (ATmega32U4, 16 MHz)
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

#ifndef HWCONFIG_H__INCLUDED
#define HWCONFIG_H__INCLUDED

#if (F_CPU != 16000000)
#error "invalid CPU clock frequency ==> should be 16 MHZ"
#endif

#if !defined(__AVR_ATmega32U4__)
#error "invalid CPU type ==> should be ATMega32u4"
#endif

#include <stdint.h>
#include <avr/io.h>

#include "devconfig.h"
#include "pinmap.h"
#include "../hwconfig_32u4.h"

#endif
