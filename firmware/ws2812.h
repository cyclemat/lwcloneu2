/*
 * LWCloneU2 – WS2811/WS2812 Bit-Bang-Treiber / bit-bang driver
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

#ifndef WS2812_H__INCLUDED
#define WS2812_H__INCLUDED

#include <stdint.h>
#include "pins.h"

/*
 * Sendet nbytes Rohdaten (3 Byte je Pixel in der Reihenfolge, die der Strip erwartet) mit
 * 800 kHz auf den Pin. Während der Übertragung sind Interrupts gesperrt (~30 µs je Pixel);
 * danach wird der Pin low gelassen, die Reset-Pause (>50 µs) muss der Aufrufer einhalten
 * (outputs.c sendet höchstens alle 10 ms).
 *
 * Sends nbytes of raw data (3 bytes per pixel in the order the strip expects) at 800 kHz.
 * Interrupts are disabled while sending (~30 µs per pixel); the pin is left low afterwards,
 * the caller has to respect the reset gap (>50 µs) – outputs.c sends at most every 10 ms.
 */
void ws2812_send(const pin_io_t *io, const uint8_t *data, uint16_t nbytes);

#endif
