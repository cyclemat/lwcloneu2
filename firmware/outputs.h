/*
 * LWCloneU2 – Ausgänge (LED-Wiz-Logik, Soft-PWM, WS2812) / outputs
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

#ifndef OUTPUTS_H__INCLUDED
#define OUTPUTS_H__INCLUDED

#include <stdint.h>

/* Timer starten, Konfiguration anwenden / start timer, apply configuration */
void outputs_init(void);

/* Pin-Belegung aus g_cfg übernehmen (auch im Betrieb) / take pin assignment from g_cfg (also at runtime) */
void outputs_apply_config(void);

/* Ein 8-Byte-LED-Wiz-Report (SBA oder PBA-Block) / one 8 byte LED-Wiz report (SBA or PBA chunk) */
void outputs_ledwiz_report(const uint8_t *p8bytes);

/* Regelmäßig aus der Hauptschleife aufrufen (WS2812-Refresh) / call regularly from the main loop */
void outputs_task(void);

/* Aktuelle Helligkeit 0..49 eines Ausgangs (für Tests) / current level 0..49 of an output */
uint8_t outputs_get_level(uint8_t index);

#endif
