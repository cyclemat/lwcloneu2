/*
 * LWCloneU2 – Laufzeit-Konfiguration / runtime configuration
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

/*
 * Die gesamte Pin-Belegung (Ausgänge, Eingänge, Analog-Achsen) liegt im EEPROM und wird
 * über USB gesetzt (siehe proto.h). Diese Datei ist die einzige Beschreibung des Formats –
 * das Konfigurations-Tool (tools/lwcconfig-web) spiegelt genau dieses Layout.
 *
 * The whole pin assignment (outputs, inputs, analog axes) lives in EEPROM and is set via
 * USB (see proto.h). This file is the single description of the layout – the configuration
 * tool (tools/lwcconfig-web) mirrors it exactly.
 *
 * Alle Mehrbyte-Werte sind little-endian. / All multi-byte values are little-endian.
 */

#ifndef CONFIG_H__INCLUDED
#define CONFIG_H__INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <hwconfig.h>

#define CFG_MAGIC0        'L'
#define CFG_MAGIC1        'W'
#define CFG_VERSION       1

/* Tabellengrößen; Boards mit wenig RAM dürfen sie in pinmap.h verkleinern. Die Werte stehen im
 * Kopf der Konfiguration, das Tool liest das Layout daraus.
 * Table sizes; boards with little RAM may shrink them in pinmap.h. The values are stored in the
 * configuration header, the tool derives the layout from them. */
#define CFG_NUM_OUTPUTS   32    /* LED-Wiz hat 32 Ausgänge / LED-Wiz has 32 outputs */
#if !defined(CFG_NUM_INPUTS)
#define CFG_NUM_INPUTS    32
#endif
#if !defined(CFG_NUM_AXES)
#define CFG_NUM_AXES      8
#endif

#define CFG_PIN_NONE      0xFF  /* Pin nicht belegt / pin not assigned */

/* Ausgangstypen / output types */
enum {
	OUT_NONE   = 0,
	OUT_GPIO   = 1,   /* Soft-PWM auf einem Pin / soft PWM on a pin                   */
	OUT_WS2812 = 2,   /* Kanal auf dem WS2811/WS2812-Strip / channel on the LED strip */
};

/* Flags für OUT_GPIO / flags for OUT_GPIO */
#define OUTF_INVERT   0x01    /* aktiv low (z. B. LED gegen VCC) / active low */

/* Farbkanal für OUT_WS2812 / color channel for OUT_WS2812 */
enum {
	WS_CH_RED   = 0,
	WS_CH_GREEN = 1,
	WS_CH_BLUE  = 2,
	WS_CH_WHITE = 3,  /* alle drei Kanäle gleich / all three channels equal */
};

/* Farbreihenfolge des Strips / color order of the strip */
enum {
	WS_ORDER_GRB = 0, /* WS2812, WS2812B (Standard / default) */
	WS_ORDER_RGB = 1, /* viele WS2811-Treiber / many WS2811 drivers */
	WS_ORDER_BRG = 2,
};

/* Flags für Eingänge / flags for inputs */
#define INF_ACTIVE_HIGH  0x01  /* Taster schaltet gegen VCC, kein Pull-up / switch to VCC, no pull-up */

/* Joystick-Achsen / joystick axes */
enum {
	AXIS_X  = 0,
	AXIS_Y  = 1,
	AXIS_Z  = 2,
	AXIS_RZ = 3,
	NUM_AXES_PER_JOY = 4,
};

/* Flags für Achsen / flags for axes */
#define AXF_INVERT    0x01

typedef struct {
	uint8_t type;      /* OUT_NONE / OUT_GPIO / OUT_WS2812                                        */
	uint8_t a;         /* GPIO: Pin-Index          WS2812: erster Pixel / first pixel             */
	uint8_t b;         /* GPIO: Flags (OUTF_*)     WS2812: Anzahl Pixel / number of pixels (>= 1)  */
	uint8_t c;         /* GPIO: -                  WS2812: Farbkanal WS_CH_* / color channel       */
} cfg_output_t;

typedef struct {
	uint8_t pin;        /* Pin-Index oder CFG_PIN_NONE                                           */
	uint8_t func;       /* Funktion (keydefs.h: KEY_*, MOD_*, AC_*, MB_*, J1_* .. J4_*, SF_*)     */
	uint8_t shift_func; /* Funktion bei gedrückter Shift-Taste / function while shift is held    */
	uint8_t flags;      /* INF_*                                                                 */
} cfg_input_t;

typedef struct {
	uint8_t  pin;       /* Pin-Index (muss ADC-fähig sein) / pin index (must be ADC capable)     */
	uint8_t  joy;       /* 0..3 = Joystick 1..4                                                  */
	uint8_t  axis;      /* AXIS_*                                                                */
	uint8_t  flags;     /* AXF_*                                                                 */
	uint16_t min;       /* Rohwert (0..1023) für -127 / raw value for -127                        */
	uint16_t center;    /* Rohwert für 0 / raw value for 0                                        */
	uint16_t max;       /* Rohwert für +127 / raw value for +127                                  */
	uint8_t  deadzone;  /* Totzone um die Mitte, 0..127 (Ausgabeeinheiten) / dead zone, 0..127   */
	uint8_t  reserved;
} cfg_axis_t;

typedef struct {
	/* Kopf / header (8 Byte) */
	uint8_t  magic[2];  /* 'L','W' */
	uint8_t  version;   /* CFG_VERSION */
	uint8_t  board;     /* BOARD_ID (pins.h) – Konfiguration ist boardgebunden / config is board specific */
	uint16_t size;      /* sizeof(config_t) */
	uint16_t crc;       /* CRC-16/CCITT über alles ab Offset 8 / over everything from offset 8 */

	/* Allgemein / general (8 Byte) */
	uint8_t  flags;
	uint8_t  n_out;     /* CFG_NUM_OUTPUTS – Anzahl Einträge in out[]  */
	uint8_t  n_in;      /* CFG_NUM_INPUTS  – Anzahl Einträge in in[]   */
	uint8_t  n_axes;    /* CFG_NUM_AXES    – Anzahl Einträge in axes[] */
	uint8_t  reserved0[4];

	/* WS2811/WS2812-Strip (4 Byte) */
	struct {
		uint8_t pin;      /* Daten-Pin oder CFG_PIN_NONE / data pin */
		uint8_t count;    /* Anzahl Pixel (<= WS2812_MAX_PIXELS) / number of pixels */
		uint8_t order;    /* WS_ORDER_* */
		uint8_t reserved;
	} ws;

	cfg_output_t out[CFG_NUM_OUTPUTS];   /* 4 Byte je Eintrag  / per entry */
	cfg_input_t  in[CFG_NUM_INPUTS];     /* 4 Byte je Eintrag  / per entry */
	cfg_axis_t   axes[CFG_NUM_AXES];     /* 12 Byte je Eintrag / per entry */
} config_t;                              /* Standard / default: 20 + 128 + 128 + 96 = 372 Byte */

/* Die aktive Konfiguration im RAM. Schreibzugriffe nur über proto.c / config_*().
 * The active configuration in RAM. Write only through proto.c / config_*(). */
extern config_t g_cfg;

/* EEPROM laden (bei Fehler: Werkseinstellungen) / load from EEPROM (defaults on error) */
void config_init(void);

/* Werkseinstellungen des Boards in g_cfg laden (nicht speichern) / load board defaults into g_cfg (not saved) */
void config_load_defaults(void);

/* g_cfg prüfen, CRC setzen und ins EEPROM schreiben / validate, set CRC and write to EEPROM */
bool config_save(void);

/* Plausibilität prüfen (Pins im Bereich, Pixel im Bereich, ...) / sanity check */
bool config_validate(const config_t *cfg);

uint16_t config_crc16(const uint8_t *p, uint16_t n);

#endif
