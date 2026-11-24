/*
 * LWCloneU2 – Eingänge: Tasten, Joysticks, Maus, Analog-Achsen / inputs
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

/* Name: hid_input.h
 * Project: V-USB Mame Panel
 * Author: Andreas Oberdorfer
 * Creation Date: 2009-09-19
 * Copyright 2009 - 2011 Andreas Oberdorfer
 * License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
 */

#ifndef PANEL_H__INCLUDED
#define PANEL_H__INCLUDED

#include <stdint.h>


/* Report-IDs der Panel-Schnittstelle (descriptors.c) / report ids of the panel interface */
enum ReportIds
{
	ID_Unknown = 0,
	ID_Keyboard,
	ID_Consumer,
	ID_Joystick1,
	ID_Joystick2,
	ID_Joystick3,
	ID_Joystick4,
	ID_Mouse
};

/* Joystick-Report: [id, x, y, z, rz, buttons]  (Achsen int8 -127..127) */
#define JOYSTICK_REPORT_SIZE 6
#define KEYBOARD_REPORT_SIZE 8
#define CONSUMER_REPORT_SIZE 2
#define MOUSE_REPORT_SIZE    4

void panel_init(void);

/* Pin-Belegung aus g_cfg übernehmen (auch im Betrieb) / take pin assignment from g_cfg */
void panel_apply_config(void);

/* Nächsten fälligen HID-Report liefern; 0 wenn nichts zu senden / next pending HID report; 0 if none */
uint8_t panel_get_report(uint8_t **ppdata);

/* Rohwert 0..1023 einer konfigurierten Achse (0 wenn keine) / raw value of a configured axis */
uint16_t panel_get_axis_raw(uint8_t axis_index);

/* Bitmaske gedrückter (entprellter) Eingänge 0..31 / bit mask of pressed (debounced) inputs */
uint32_t panel_get_input_mask(void);

#endif  // PANEL_H__INCLUDED
