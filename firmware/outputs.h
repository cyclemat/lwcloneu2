/*
 * LWCloneU2 – Ausgänge (LED-Wiz-Logik, Soft-PWM auf GPIO) / outputs
 *
 * Copyright (C) 2013 Andreas Dittrich <lwcloneu2@cithraidt.de>
 * Copyright (C) 2026 cyclemat
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without either the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#ifndef OUTPUTS_H__INCLUDED
#define OUTPUTS_H__INCLUDED

#include <stdint.h>

#define OUTPUTS_MAX_LEVEL   49    /* Stufen der Soft-PWM / soft PWM steps (0 = aus / off) */

/* Timer starten, Konfiguration anwenden / start timer, apply configuration */
void outputs_init(void);

/* Pin-Belegung aus g_cfg übernehmen (auch im Betrieb) / take pin assignment from g_cfg (also at runtime) */
void outputs_apply_config(void);

/* Ein 8-Byte-LED-Wiz-Report (SBA oder PBA-Block) / one 8 byte LED-Wiz report (SBA or PBA chunk) */
void outputs_ledwiz_report(const uint8_t *p8bytes);

/* Aktuelle Helligkeit 0..OUTPUTS_MAX_LEVEL eines Ausgangs (für strip.c, Tests)
 * current level 0..OUTPUTS_MAX_LEVEL of an output (for strip.c, tests) */
uint8_t outputs_get_level(uint8_t index);

#endif
