/*
 * LWCloneU2 – WS2811/WS2812-Strip (eigenes Modul) / LED strip (separate module)
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
 * Ein WS2811/WS2812-Strip an einem konfigurierbaren Daten-Pin, zusätzlich zu den 32 LED-Wiz-
 * Ausgängen und ohne Einfluss auf sie. Zwei Betriebsarten (g_cfg.ws.mode):
 *
 *   STRIP_MODE_DIRECT  Der PC setzt Pixel über die USB-Befehle 0xB0..0xB3 (proto.h) und löst mit
 *                      STRIP_SHOW die Ausgabe aus. Für eigene Programme, z. B. per LWZ_RAWWRITE.
 *   STRIP_MODE_MIRROR  Der Strip spiegelt LED-Wiz-Ausgänge: je Eintrag in g_cfg.mirror[] leuchtet
 *                      ein Pixelbereich in einer Farbe, die Helligkeit folgt dem Ausgang. Damit
 *                      funktioniert der Strip sofort mit DOF, LedBlinky usw.
 *
 * Der Strip wird höchstens alle STRIP_REFRESH_MS beschrieben; während der Übertragung sind
 * Interrupts gesperrt (~30 µs je Pixel, siehe ws2812.h).
 *
 * One WS2811/WS2812 strip on a configurable data pin, in addition to the 32 LED-Wiz outputs and
 * without affecting them. Two modes: DIRECT (host sets pixels via USB commands 0xB0..0xB3 and
 * triggers output with STRIP_SHOW) and MIRROR (pixel ranges follow LED-Wiz outputs in a color).
 * The strip is written at most every STRIP_REFRESH_MS; interrupts are blocked while sending.
 */

#ifndef STRIP_H__INCLUDED
#define STRIP_H__INCLUDED

#include <stdint.h>
#include <stdbool.h>

#define STRIP_REFRESH_MS   10

/* Konfiguration anwenden (Pin, Länge, Betriebsart) / apply configuration */
void strip_init(void);
void strip_apply_config(void);

/* Regelmäßig aus der Hauptschleife aufrufen: spiegelt Ausgänge, schreibt den Strip
 * call regularly from the main loop: mirrors outputs, writes the strip */
void strip_task(void);

/* Ein 8-Byte-Report mit Befehl 0xB0..0xBF (proto.h). Gibt true zurück, wenn reply8 gefüllt wurde
 * und sofort gesendet werden soll (STRIP_SHOW antwortet später aus strip_task()).
 * One 8 byte report with command 0xB0..0xBF. Returns true if reply8 was filled and has to be sent
 * now (STRIP_SHOW replies later from strip_task()). */
bool strip_handle_report(const uint8_t *p8, uint8_t *reply8);

#endif
