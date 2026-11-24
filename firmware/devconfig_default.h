/*
 * LWCloneU2 – USB-Gerätekonfiguration (Build-Schalter) / USB device configuration (build switches)
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
 * Welche USB-Schnittstellen das Gerät anmeldet. Was ein Pin tut, steht nicht mehr hier,
 * sondern in der Laufzeit-Konfiguration (config.h, Konfigurations-Tool).
 *
 * Which USB interfaces the device enumerates. What a pin does is no longer decided here
 * but in the runtime configuration (config.h, configuration tool).
 *
 * Bei Boards mit zwei MCUs (Uno, Mega) müssen beide mit derselben devconfig.h gebaut werden.
 * Boards with two MCUs (Uno, Mega) must build both with the same devconfig.h.
 */

#ifndef DEVCONFIG_DEFAULT_H__INCLUDED
#define DEVCONFIG_DEFAULT_H__INCLUDED

/* LED-Wiz-kompatible Ausgangsschnittstelle / LED-Wiz compatible output interface */
#define ENABLE_LED_DEVICE

/* Tastatur, Consumer, Maus und Joysticks / keyboard, consumer, mouse and joysticks */
#define ENABLE_PANEL_DEVICE

/* Anzahl Joysticks (1..4) im HID-Deskriptor / number of joysticks (1..4) in the HID descriptor */
#define NUM_JOYSTICKS 4

#endif
