/*
 * LWCloneU2 (Due) – WS2811/WS2812 Bit-Bang-Treiber mit Zyklenzähler (DWT) bei 84 MHz
 * LWCloneU2 (Due) – WS2811/WS2812 bit-bang driver using the cycle counter (DWT) at 84 MHz
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
 * Zeitraster 800 kHz (1,25 µs je Bit): 0-Bit ~0,35 µs high, 1-Bit ~0,75 µs high – nach Datenblatt
 * WS2812B / WS2811 (high speed mode). Die Zeiten werden mit dem Cortex-M3-Zyklenzähler gemessen,
 * sind also unabhängig von Compiler-Optimierung. Interrupts sind währenddessen gesperrt
 * (~30 µs je Pixel, bei 255 Pixeln ~7,7 ms). Der Pegel ist 3,3 V – viele Strips brauchen einen
 * Pegelwandler oder vertragen 3,3 V am Dateneingang nur bei knapper Versorgung.
 *
 * Timing 800 kHz (1.25 µs per bit): 0 bit ~0.35 µs high, 1 bit ~0.75 µs high – per the WS2812B /
 * WS2811 data sheets (high speed mode). Times are measured with the Cortex-M3 cycle counter, so
 * they do not depend on compiler optimization. Interrupts are disabled meanwhile (~30 µs per pixel,
 * ~7.7 ms for 255 pixels). The level is 3.3 V – many strips need a level shifter.
 */

#include <Arduino.h>
#include "lwc_shared.h"


#define CYCLES_PER_US   (VARIANT_MCK / 1000000UL)              /* 84 */
#define T_PERIOD        (CYCLES_PER_US * 125 / 100)            /* 1,25 µs = 105 Takte */
#define T0H             (CYCLES_PER_US * 35 / 100)             /* 0,35 µs =  29 Takte */
#define T1H             (CYCLES_PER_US * 75 / 100)             /* 0,75 µs =  63 Takte */

/* Die CMSIS-Version im Arduino-SAM-Core kennt die DWT-Register nicht – Adressen nach ARMv7-M-Referenz
 * The CMSIS version in the Arduino SAM core lacks the DWT registers – addresses per the ARMv7-M reference */
#define LWC_DEMCR        (*(volatile uint32_t *)0xE000EDFCu)   /* Debug Exception and Monitor Control */
#define LWC_DWT_CTRL     (*(volatile uint32_t *)0xE0001000u)
#define LWC_DWT_CYCCNT   (*(volatile uint32_t *)0xE0001004u)
#define LWC_DEMCR_TRCENA (1u << 24)
#define LWC_DWT_CYCCNTENA (1u << 0)

static void dwt_enable(void)
{
	LWC_DEMCR    |= LWC_DEMCR_TRCENA;
	LWC_DWT_CTRL |= LWC_DWT_CYCCNTENA;
}

static inline uint32_t cyc(void)
{
	return LWC_DWT_CYCCNT;
}


extern "C" void ws2812_send(const pin_io_t *io, const uint8_t *data, uint16_t nbytes)
{
	if (io == NULL || data == NULL || nbytes == 0)
		return;

	volatile uint32_t * const sodr = io->reg;
	volatile uint32_t * const codr = io->reg + 1;
	uint32_t const mask = io->mask;

	dwt_enable();

	uint32_t const primask = __get_PRIMASK();
	__disable_irq();

	uint32_t t = cyc();

	while (nbytes--)
	{
		uint8_t b = *data++;

		for (uint8_t i = 0; i < 8; i++, b <<= 1)
		{
			uint32_t const th = (b & 0x80) ? T1H : T0H;

			*sodr = mask;
			while ((uint32_t)(cyc() - t) < th) {}
			*codr = mask;
			while ((uint32_t)(cyc() - t) < T_PERIOD) {}
			t += T_PERIOD;
		}
	}

	__set_PRIMASK(primask);
}
