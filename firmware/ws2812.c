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

/*
 * Zeitraster (800 kHz, 1,25 µs je Bit):   0-Bit: ~0,35 µs high   1-Bit: ~0,8 µs high
 * Eigene Umsetzung nach Datenblatt WS2812B / WS2811 (high speed mode).
 * Timing (800 kHz, 1.25 µs per bit):      0 bit: ~0.35 µs high   1 bit: ~0.8 µs high
 * Own implementation following the WS2812B / WS2811 data sheets.
 */

#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "ws2812.h"


void ws2812_send(const pin_io_t *io, const uint8_t *data, uint16_t nbytes)
{
	if (io == NULL || data == NULL || nbytes == 0)
		return;

	volatile uint8_t * const port = io->port;

	uint8_t const sreg = SREG;
	cli();

	/* Beide Portzustände vorberechnen – mit gesperrten Interrupts ändert niemand die anderen Bits.
	 * Precompute both port states – with interrupts disabled nobody touches the other bits. */
	uint8_t const hi = (uint8_t)(*port | io->mask);
	uint8_t const lo = (uint8_t)(*port & (uint8_t)~io->mask);

	uint8_t cur, bits;

#if (F_CPU == 16000000UL)

	/* 20 Takte je Bit (19 bei 1-Bit).  T0H = 5 Takte (312 ns), T1H = 13 Takte (812 ns) */
	__asm__ volatile (
		"1:"                        "\n\t"
		"ld   %[cur], %a[data]+"    "\n\t"
		"ldi  %[bits], 8"           "\n\t"
		"2:"                        "\n\t"
		"st   %a[port], %[hi]"      "\n\t"   /* 1-2   Pin high                 */
		"nop"                       "\n\t"   /* 3                              */
		"nop"                       "\n\t"   /* 4                              */
		"sbrs %[cur], 7"            "\n\t"   /* 5     (5-6 wenn Bit = 1)       */
		"st   %a[port], %[lo]"      "\n\t"   /* 6-7   Bit 0: Pin low           */
		"nop"                       "\n\t"   /* 8  /  7                        */
		"nop"                       "\n\t"
		"nop"                       "\n\t"
		"nop"                       "\n\t"
		"nop"                       "\n\t"
		"nop"                       "\n\t"
		"nop"                       "\n\t"   /* 14 /  13                       */
		"st   %a[port], %[lo]"      "\n\t"   /* 15-16 / 14-15  Bit 1: Pin low  */
		"lsl  %[cur]"               "\n\t"   /* 17 / 16                        */
		"dec  %[bits]"              "\n\t"   /* 18 / 17                        */
		"brne 2b"                   "\n\t"   /* 19-20 / 18-19                  */
		"sbiw %[n], 1"              "\n\t"
		"brne 1b"                   "\n\t"
		: [cur] "=&d" (cur), [bits] "=&d" (bits), [data] "+e" (data), [n] "+w" (nbytes)
		: [port] "e" (port), [hi] "r" (hi), [lo] "r" (lo)
	);

#elif (F_CPU == 8000000UL)

	/* 12 Takte je Bit (11 bei 1-Bit).  T0H = 3 Takte (375 ns), T1H = 5 Takte (625 ns) */
	__asm__ volatile (
		"1:"                        "\n\t"
		"ld   %[cur], %a[data]+"    "\n\t"
		"ldi  %[bits], 8"           "\n\t"
		"2:"                        "\n\t"
		"st   %a[port], %[hi]"      "\n\t"   /* 1-2   Pin high                 */
		"sbrs %[cur], 7"            "\n\t"   /* 3     (3-4 wenn Bit = 1)       */
		"st   %a[port], %[lo]"      "\n\t"   /* 4-5   Bit 0: Pin low           */
		"nop"                       "\n\t"   /* 6  /  5                        */
		"st   %a[port], %[lo]"      "\n\t"   /* 7-8 / 6-7  Bit 1: Pin low      */
		"lsl  %[cur]"               "\n\t"
		"dec  %[bits]"              "\n\t"
		"brne 2b"                   "\n\t"
		"sbiw %[n], 1"              "\n\t"
		"brne 1b"                   "\n\t"
		: [cur] "=&d" (cur), [bits] "=&d" (bits), [data] "+e" (data), [n] "+w" (nbytes)
		: [port] "e" (port), [hi] "r" (hi), [lo] "r" (lo)
	);

#else
	#error "ws2812.c: nur 8 MHz oder 16 MHz unterstützt / only 8 MHz or 16 MHz supported"
#endif

	SREG = sreg;
}
