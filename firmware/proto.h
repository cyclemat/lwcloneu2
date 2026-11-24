/*
 * LWCloneU2 – USB-Befehle (LED-Wiz + Konfiguration) / USB commands
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
 * Alle Befehle sind 8-Byte-Output-Reports auf der LED-Wiz-HID-Schnittstelle, Antworten sind
 * 8 Byte Nutzdaten in einem 64-Byte-Input-Report derselben Schnittstelle (Rest 0).
 * Byte 0 ist immer der Befehl; Antworten wiederholen ihn in Byte 0.
 *
 * All commands are 8 byte output reports on the LED-Wiz HID interface, replies are 8 bytes of
 * payload inside a 64 byte input report of the same interface (rest 0). Byte 0 is always the
 * command; replies repeat it in byte 0.
 *
 *  64   SBA        [64, bank0, bank1, bank2, bank3, speed, 0, 0]              (LED-Wiz)
 *  --   PBA        4 × [v0..v7]  Profilwerte / profile values                 (LED-Wiz)
 *  65   SETID      [65, id-1, FF, FF, FF, FF, FF, ~(id-1)]  → Neustart / reboot
 *  66   DFU        [66, 0, FF, FF, FF, FF, FF, 0xFF]        → Bootloader
 *
 *  A0   CFG_INFO     [A0]                     → [A0, proto, board, fw_major, fw_minor, size_lo, size_hi, num_pins]
 *  A1   CFG_READ     [A1, off_lo, off_hi]     → [A1, off_lo, off_hi, d0, d1, d2, d3, d4]
 *  A2   CFG_WRITE    [A2, off_lo, off_hi, d0..d4] → [A2, off_lo, off_hi, status]       (nur RAM / RAM only)
 *  A3   CFG_APPLY    [A3]                     → [A3, status]      sofort anwenden / apply now ("on the fly")
 *  A4   CFG_SAVE     [A4]                     → [A4, status]      prüfen, anwenden, EEPROM
 *  A5   CFG_DEFAULTS [A5]                     → [A5, status]      Werkseinstellungen in RAM + anwenden
 *  A6   CFG_LIVE     [A6, idx]                → idx 0..2: [A6, idx, a_lo, a_hi, b_lo, b_hi, c_lo, c_hi]
 *                                                  Rohwerte (0..1023) der Achsen 3·idx .. 3·idx+2
 *                                               idx 3:    [A6, 3, in0, in1, in2, in3, 0, 0]
 *                                                  Bitmaske gedrückter Eingänge 0..31 (entprellt)
 *  A7   CFG_PININFO  [A7, idx]                → [A7, idx, p0, b0, p1, b1, p2, b2]
 *                                                  3 Pins ab idx: p = Port (0=A..10=L), b = Bit | 0x80 wenn ADC-fähig
 *                                                  p = 0xFF wenn kein Pin mehr / if no more pins
 *
 *  WS2811/WS2812-Strip (strip.c), nur in STRIP_MODE_DIRECT wirksam / LED strip, STRIP_MODE_DIRECT only:
 *  B0   STRIP_INFO   [B0]                     → [B0, count, max_pixels, mode, order, 0, 0, 0]
 *  B1   STRIP_SET    [B1, idx, r, g, b, r, g, b]   2 Pixel ab idx setzen (nur Puffer) / set 2 pixels (buffer only)
 *  B2   STRIP_FILL   [B2, idx, n, r, g, b]         n Pixel ab idx füllen, n = 0: bis zum Ende / fill, 0 = to the end
 *  B3   STRIP_SHOW   [B3]                     → [B3, status]   Puffer an den Strip senden; die Antwort kommt,
 *                                                  sobald der Strip geschrieben ist – vor der nächsten
 *                                                  Pixel-Folge darauf warten (Brücke Mega/Uno: UART-Puffer).
 *                                                  send buffer to the strip; the reply arrives once the strip
 *                                                  has been written – wait for it before the next pixel burst.
 *
 *  status: 0 = ok, 1 = ungültig / invalid, 2 = Fehler / error
 */

#ifndef PROTO_H__INCLUDED
#define PROTO_H__INCLUDED

#include <stdint.h>
#include <stdbool.h>

#define PROTO_VERSION      2
#define FW_VERSION_MAJOR   2
#define FW_VERSION_MINOR   1

enum {
	CMD_SBA          = 64,
	CMD_SETID        = 65,
	CMD_DFU          = 66,

	CMD_CFG_INFO     = 0xA0,
	CMD_CFG_READ     = 0xA1,
	CMD_CFG_WRITE    = 0xA2,
	CMD_CFG_APPLY    = 0xA3,
	CMD_CFG_SAVE     = 0xA4,
	CMD_CFG_DEFAULTS = 0xA5,
	CMD_CFG_LIVE     = 0xA6,
	CMD_CFG_PININFO  = 0xA7,

	CMD_CFG_FIRST    = 0xA0,
	CMD_CFG_LAST     = 0xAF,

	CMD_STRIP_INFO   = 0xB0,
	CMD_STRIP_SET    = 0xB1,
	CMD_STRIP_FILL   = 0xB2,
	CMD_STRIP_SHOW   = 0xB3,

	CMD_STRIP_FIRST  = 0xB0,
	CMD_STRIP_LAST   = 0xBF,
};

enum {
	STATUS_OK      = 0,
	STATUS_INVALID = 1,
	STATUS_ERROR   = 2,
};

#define PROTO_REPORT_SIZE 8

/* Ist das ein Konfigurations-/Strip-Befehl (statt LED-Wiz-Daten)? Antworten tragen denselben
 * Befehlscode in Byte 0 – die Brücke (main_usb.c) erkennt sie daran.
 * Is this a configuration/strip command (instead of LED-Wiz data)? Replies carry the same command
 * code in byte 0 – the bridge (main_usb.c) recognizes them by it. */
static inline bool proto_is_cfg_cmd(uint8_t b0) { return b0 >= CMD_CFG_FIRST && b0 <= CMD_STRIP_LAST; }

/* Report auf dem MCU verarbeiten, der Ausgänge und Konfiguration besitzt.
 * Process a report on the MCU that owns outputs and configuration. */
void proto_handle_report(const uint8_t *p8);

/* Antwort (8 Byte) einreihen – für Module, die verzögert antworten (strip.c)
 * queue a reply (8 bytes) – for modules that reply later (strip.c) */
void proto_push_reply(const uint8_t *p8);

/* Nächste Antwort (8 Byte) holen, false wenn keine / fetch next reply, false if none */
bool proto_pop_reply(uint8_t *dst8);

#endif
