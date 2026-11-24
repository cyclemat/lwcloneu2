/*
 * LWCloneU2 – Hardware-Einstellungen Arduino Due (SAM3X8E, nativer USB-Port)
 * LWCloneU2 – hardware setup Arduino Due (SAM3X8E, native USB port)
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
 * Diese Datei ersetzt hwconfig.h/pinmap.h/devconfig.h der AVR-Boards. Sie wird von den gemeinsamen
 * C-Modulen (Ordner ../firmware) und vom Due-Code eingebunden und darf deshalb keine Arduino-Header ziehen.
 *
 * This file replaces hwconfig.h/pinmap.h/devconfig.h of the AVR boards. It is included by the shared
 * C modules (folder ../firmware) and by the Due code, so it must not pull in Arduino headers.
 *
 * Pin-Index = Arduino-Pin-Nummer des Due / pin index = Arduino pin number of the Due:
 *   0..53  D0..D53 (D0/D1 = UART zum 16U2, gesperrt / reserved)     54..65  A0..A11 (ADC)
 *   66 DAC0   67 DAC1   68 CANRX   69 CANTX   70 SDA1   71 SCL1   72 RX-LED   73 TX-LED
 * Alle Pins 3,3 V! / All pins are 3.3 V!
 */

#ifndef HWCONFIG_DUE_H__INCLUDED
#define HWCONFIG_DUE_H__INCLUDED

#include <stdint.h>
#include <stdbool.h>

/* Schnittstellen / interfaces */
#define ENABLE_LED_DEVICE
#define ENABLE_PANEL_DEVICE
#define ENABLE_STRIP_SERIAL          /* COM-Port für DOF (TeensyStripController-Protokoll) / COM port for DOF */
#define NUM_JOYSTICKS        4

#define BOARD_ID             BOARD_ARDUINO_DUE
#define WS2812_MAX_PIXELS    255     /* Protokollgrenze (uint8) – RAM wäre für viel mehr da / protocol limit (uint8) */
#define LWC_NUM_PINS         74

#define PANEL_TASK                   /* panel.c aktiv / panel.c active */
#define BOARD_HAS_ADC        1       /* 12 Kanäle A0..A11, 12 Bit → auf 10 Bit gekürzt / 12 channels, 12 bit → 10 bit */
#define OUTPUTS_TICK_EXTERNAL 1      /* outputs_tick() kommt aus dem TC-Timer in hw_due.cpp / from the TC timer */

/* Werksbelegung: die 32 LED-Wiz-Ausgänge auf D22..D53 (der doppelreihige Header)
 * factory defaults: the 32 LED-Wiz outputs on D22..D53 (the double-row header) */
#define BOARD_DEFAULT_OUTPUTS \
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, \
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53

#ifdef __cplusplus
extern "C" {
#endif

/* Soft-PWM-Takt 200 µs (TC1 Kanal 0) → outputs_tick() / soft PWM clock 200 µs → outputs_tick() */
void led_timer_init(void);

/* ADC im Freilauf; Kanalnummern = SAM-ADC-Kanal (pins_adc_mux) / free-running ADC; channel numbers = SAM ADC channel */
void     ADC_init(void);
void     ADC_select(const uint8_t *channels, uint8_t n);   /* genau diese Kanäle wandeln / convert exactly these */
uint16_t ADC_read(uint8_t channel);                        /* 0..1023 */

#ifdef __cplusplus
}
#endif

#endif
