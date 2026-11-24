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

/* Name: hid_input.c
 * Project: V-USB Mame Panel
 * Author: Andreas Oberdorfer
 * Creation Date: 2009-09-19
 * Copyright 2009 - 2011 Andreas Oberdorfer
 * License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
 */

/*
 * Entprellung und Report-Aufbau stammen aus dem V-USB-Mame-Panel; die Pin-Zuordnung kommt
 * jetzt zur Laufzeit aus g_cfg (config.h) statt aus Compile-Zeit-Tabellen.
 *
 * Debouncing and report assembly come from the V-USB Mame Panel; the pin mapping now comes
 * from g_cfg (config.h) at runtime instead of compile time tables.
 */

#include <string.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include <hwconfig.h>
#include "config.h"
#include "pins.h"
#include "panel.h"
#include "keydefs.h"
#include "clock.h"


#if !defined(PANEL_TASK)

void panel_init(void) {}
void panel_apply_config(void) {}
uint8_t panel_get_report(uint8_t **ppdata) { (void)ppdata; return 0; }
uint16_t panel_get_axis_raw(uint8_t axis_index) { (void)axis_index; return 0; }
uint32_t panel_get_input_mask(void) { return 0; }

#else


#if !defined(NUM_JOYSTICKS)
#define NUM_JOYSTICKS 4
#endif

#define DEBOUNCE            5
#define SCAN_INTERVAL_MS    5
#define AXIS_HYSTERESIS     2      /* Mindeständerung für neuen Report / minimum change for a new report */

#if !defined(MOUSE_DELTA)
#define MOUSE_DELTA 1
#endif


/* ---------------------------------------------------------------- digitale Eingänge / digital inputs */

typedef struct {
	pin_io_t io;
	uint8_t  func;
	uint8_t  shift_func;
	uint8_t  active_high;
} input_t;

static input_t  s_in[CFG_NUM_INPUTS];
static uint8_t  s_nin = 0;
static uint8_t  s_state[CFG_NUM_INPUTS];     /* Bit 7: gedrückt, Bits 0..6: Entprellzähler / debounce counter */
static volatile uint32_t s_input_mask = 0;

static int8_t   s_shift_idx = -1;
static int8_t   s_mx_clk = -1, s_mx_dir = -1, s_my_clk = -1, s_my_dir = -1;

static uint8_t  s_shift_key = 0;
static uint8_t  s_shift_cleanup = 0;
static uint8_t  s_need_key = 0;
static uint8_t  s_need_consumer = 0;
static uint8_t  s_need_mouse = 0;
static uint8_t  s_need_joy[MAX_JOYSTICKS];
static uint8_t  s_last_joy = 0;

static uint8_t  s_mx_last_clk = 0, s_mx_last_dir = 0;
static uint8_t  s_my_last_clk = 0, s_my_last_dir = 0;
static int8_t   s_mouse_x = 0, s_mouse_y = 0;

static uint8_t  s_report[KEYBOARD_REPORT_SIZE];


/* ---------------------------------------------------------------- Analog-Achsen / analog axes */

#if defined(BOARD_HAS_ADC)

typedef struct {
	uint8_t  mux;
	uint8_t  joy;
	uint8_t  axis;
	uint8_t  flags;
	uint16_t min;
	uint16_t center;
	uint16_t max;
	uint8_t  deadzone;
} axis_t;

static axis_t           s_ax[CFG_NUM_AXES];
static volatile uint8_t s_nax = 0;
static volatile uint16_t s_adc_raw[CFG_NUM_AXES];

#endif

static int8_t s_axis_val[MAX_JOYSTICKS][NUM_AXES_PER_JOY];   /* aktuell berechnet / currently computed */
static int8_t s_axis_sent[MAX_JOYSTICKS][NUM_AXES_PER_JOY];  /* zuletzt gemeldet / last reported */
static uint8_t s_joy_has_analog[MAX_JOYSTICKS];


/* ---------------------------------------------------------------- Hilfsfunktionen / helpers */

#define IsKeyDown(index)  (s_state[index] & 0x80)

static uint8_t GetKey(uint8_t i)
{
	return s_shift_key ? s_in[i].shift_func : s_in[i].func;
}

static bool IsKeyboardCode(uint8_t key) { return (key >= KEY_A) && (key <= MOD_RightGUI); }
static bool IsModifierCode(uint8_t key) { return (key >= MOD_LeftControl) && (key <= MOD_RightGUI); }
static bool IsConsumerCode(uint8_t key) { return (key >= AC_VolumeUp) && (key <= AC_Mute); }
static bool IsMouseButtonCode(uint8_t key) { return (key >= MB_Left) && (key <= MB_Middle); }
static bool IsJoystickCode(uint8_t key, uint8_t joy)
{
	key -= (uint8_t)(joy * NR_OF_EVENTS_PER_JOY);
	return (key >= J1_Left) && (key < (J1_Left + NR_OF_EVENTS_PER_JOY));
}

static bool NeedJoystickUpdate(void)
{
	for (uint8_t i = 0; i < NUM_JOYSTICKS; i++)
		if (s_need_joy[i])
			return true;

	return false;
}


/* ---------------------------------------------------------------- Konfiguration / configuration */

void panel_apply_config(void)
{
	/* alte Pull-ups lösen / release old pull-ups */
	for (uint8_t i = 0; i < s_nin; i++)
		pins_mode_input(&s_in[i].io, false);

	s_nin = 0;
	s_shift_idx = -1;
	s_mx_clk = s_mx_dir = s_my_clk = s_my_dir = -1;
	s_shift_key = 0;
	s_shift_cleanup = 0;
	memset(s_state, 0x00, sizeof(s_state));
	s_input_mask = 0;

	for (uint8_t i = 0; i < CFG_NUM_INPUTS; i++)
	{
		cfg_input_t const * const c = &g_cfg.in[i];
		input_t * const in = &s_in[s_nin];

		if (c->pin == CFG_PIN_NONE || !pins_resolve(c->pin, &in->io))
			continue;

		in->func        = c->func;
		in->shift_func  = c->shift_func;
		in->active_high = (c->flags & INF_ACTIVE_HIGH) ? 1 : 0;

		pins_mode_input(&in->io, !in->active_high);

		switch (c->func)
		{
		case SF_Shift:     s_shift_idx = (int8_t)s_nin; break;
		case SF_MouseXClk: s_mx_clk    = (int8_t)s_nin; break;
		case SF_MouseXDir: s_mx_dir    = (int8_t)s_nin; break;
		case SF_MouseYClk: s_my_clk    = (int8_t)s_nin; break;
		case SF_MouseYDir: s_my_dir    = (int8_t)s_nin; break;
		default: break;
		}

		s_nin++;
	}

	/* Achsen / axes */
	memset(s_joy_has_analog, 0x00, sizeof(s_joy_has_analog));

	#if defined(BOARD_HAS_ADC)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { s_nax = 0; }

		uint8_t n = 0;

		for (uint8_t i = 0; i < CFG_NUM_AXES; i++)
		{
			cfg_axis_t const * const c = &g_cfg.axes[i];

			if (c->pin == CFG_PIN_NONE || c->joy >= MAX_JOYSTICKS || c->axis >= NUM_AXES_PER_JOY)
				continue;

			uint8_t const mux = pins_adc_mux(c->pin);
			pin_io_t io;

			if (mux == PIN_NO_ADC || !pins_resolve(c->pin, &io))
				continue;

			pins_mode_input(&io, false);   /* kein Pull-up am Poti / no pull-up on the pot */

			s_ax[n].mux      = mux;
			s_ax[n].joy      = c->joy;
			s_ax[n].axis     = c->axis;
			s_ax[n].flags    = c->flags;
			s_ax[n].min      = c->min;
			s_ax[n].center   = c->center;
			s_ax[n].max      = c->max;
			s_ax[n].deadzone = c->deadzone;
			s_adc_raw[n]     = c->center;
			s_joy_has_analog[c->joy] = 1;
			n++;
		}

		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			s_nax = n;

			if (n > 0)
			{
				ADC_setmux(s_ax[0].mux);
				ADCSRA |= (1 << ADSC);    /* Wandlungskette starten / start conversion chain */
			}
		}
	}
	#endif

	memset(s_axis_val, 0x00, sizeof(s_axis_val));
	memset(s_axis_sent, 0x00, sizeof(s_axis_sent));

	for (uint8_t j = 0; j < NUM_JOYSTICKS; j++)
		s_need_joy[j] = 1;    /* Ausgangszustand melden / report initial state */

	s_need_key = 1;
	s_need_consumer = 1;
}

void panel_init(void)
{
	#if defined(BOARD_HAS_ADC)
	ADC_init();
	#endif

	panel_apply_config();
}


/* ---------------------------------------------------------------- ADC */

#if defined(BOARD_HAS_ADC)

ISR(ADC_vect)
{
	static uint8_t i = 0;

	uint8_t const n = s_nax;

	if (n == 0)
	{
		i = 0;
		return;         /* keine Achsen: Kette anhalten / no axes: stop the chain */
	}

	if (i < n)
		s_adc_raw[i] = ADC;

	i++;
	if (i >= n)
		i = 0;

	ADC_setmux(s_ax[i].mux);
	ADCSRA |= (1 << ADSC);
}

uint16_t panel_get_axis_raw(uint8_t axis_index)
{
	uint16_t v = 0;

	if (axis_index < CFG_NUM_AXES)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { v = s_adc_raw[axis_index]; }
	}

	return v;
}

/* Rohwert → -127..127 mit Kalibrierung, Totzone und Invertierung
 * raw value → -127..127 with calibration, dead zone and inversion */
static int8_t axis_scale(const axis_t *a, uint16_t raw)
{
	int16_t v;

	if (raw <= a->center)
	{
		uint16_t const span = a->center - a->min;
		if (span == 0) return 0;
		if (raw < a->min) raw = a->min;
		v = -(int16_t)(((uint32_t)(a->center - raw) * 127 + span / 2) / span);
	}
	else
	{
		uint16_t const span = a->max - a->center;
		if (span == 0) return 0;
		if (raw > a->max) raw = a->max;
		v = (int16_t)(((uint32_t)(raw - a->center) * 127 + span / 2) / span);
	}

	/* Totzone: innerhalb 0, außerhalb neu auf volle Breite skalieren
	 * dead zone: zero inside, rescale to full range outside */
	uint8_t const dz = a->deadzone;

	if (dz > 0 && dz < 127)
	{
		int16_t const mag = (v < 0) ? -v : v;

		if (mag <= dz)
		{
			v = 0;
		}
		else
		{
			int16_t const m = (int16_t)(((int32_t)(mag - dz) * 127 + (127 - dz) / 2) / (127 - dz));
			v = (v < 0) ? -m : m;
		}
	}

	if (v > 127) v = 127;
	if (v < -127) v = -127;

	if (a->flags & AXF_INVERT)
		v = -v;

	return (int8_t)v;
}

static void UpdateAnalogAxes(void)
{
	uint8_t const n = s_nax;

	for (uint8_t i = 0; i < n; i++)
	{
		axis_t const * const a = &s_ax[i];
		uint16_t raw;

		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { raw = s_adc_raw[i]; }

		s_axis_val[a->joy][a->axis] = axis_scale(a, raw);
	}

	for (uint8_t j = 0; j < NUM_JOYSTICKS; j++)
	{
		if (!s_joy_has_analog[j])
			continue;

		for (uint8_t k = 0; k < NUM_AXES_PER_JOY; k++)
		{
			int16_t const d = (int16_t)s_axis_val[j][k] - (int16_t)s_axis_sent[j][k];
			int16_t const mag = (d < 0) ? -d : d;
			int8_t const v = s_axis_val[j][k];

			if (mag >= AXIS_HYSTERESIS || (mag != 0 && (v == 0 || v == 127 || v == -127)))
			{
				s_need_joy[j] = 1;
				break;
			}
		}
	}
}

#else

uint16_t panel_get_axis_raw(uint8_t axis_index) { (void)axis_index; return 0; }
static void UpdateAnalogAxes(void) {}

#endif


/* ---------------------------------------------------------------- Maus-Encoder / mouse encoder */

static void MouseMove(int8_t *count, uint8_t direction)
{
	if (direction)
	{
		if (*count > -(127 - MOUSE_DELTA))
			*count -= MOUSE_DELTA;
	}
	else
	{
		if (*count < (127 - MOUSE_DELTA))
			*count += MOUSE_DELTA;
	}

	s_need_mouse = 1;
}

static void CheckEncoder(int8_t iclk, int8_t idir, uint8_t *last_clk, uint8_t *last_dir, int8_t *count)
{
	if (iclk < 0 || idir < 0)
		return;

	uint8_t const clk = s_state[(uint8_t)iclk];
	uint8_t const dir = s_state[(uint8_t)idir];

	if (clk != *last_clk)
	{
		if (dir == *last_dir)
			MouseMove(count, clk ^ dir);

		*last_clk = clk;
	}

	if (dir != *last_dir)
	{
		if (clk == *last_clk)
			MouseMove(count, !(clk ^ dir));

		*last_dir = dir;
	}
}


/* ---------------------------------------------------------------- Abtastung / scanning */

static void SetNeedUpdate(uint8_t index)
{
	uint8_t const key = GetKey(index);

	if (IsConsumerCode(key))      { s_need_consumer = 1; return; }
	if (IsKeyboardCode(key))      { s_need_key = 1; return; }
	if (IsMouseButtonCode(key))   { s_need_mouse = 1; return; }

	for (uint8_t j = 0; j < NUM_JOYSTICKS; j++)
	{
		if (IsJoystickCode(key, j))
		{
			s_need_joy[j] = 1;
			return;
		}
	}
}

static void ShiftKeyCleanUp(void)
{
	if (s_shift_idx < 0)
		return;

	if (s_shift_cleanup == 1)
	{
		s_shift_cleanup = 2;

		for (uint8_t i = 0; i < s_nin; i++)
		{
			if ((int8_t)i == s_shift_idx || s_state[i] == 0)
				continue;

			if (s_in[i].func != s_in[i].shift_func)
			{
				SetNeedUpdate(i);
				s_state[i] = 0;
			}
		}
	}

	if (s_shift_cleanup == 2)
	{
		bool const no_update = !s_need_consumer && !s_need_key && !s_need_mouse && !NeedJoystickUpdate();

		if (no_update)
		{
			s_shift_cleanup = 0;
			s_shift_key = IsKeyDown((uint8_t)s_shift_idx) ? 1 : 0;
		}
	}
}

static void SetInputCount(uint8_t index, uint8_t condition)
{
	/* Encoder-Signale werden nicht entprellt / encoder signals are not debounced */
	if ((int8_t)index == s_mx_clk || (int8_t)index == s_mx_dir ||
	    (int8_t)index == s_my_clk || (int8_t)index == s_my_dir)
	{
		s_state[index] = condition;
		return;
	}

	uint8_t changed = 0;
	uint8_t count = s_state[index];
	uint8_t state = count & 0x80;
	count &= 0x7f;

	if (condition)
	{
		if (count <= DEBOUNCE)
		{
			if ((count == DEBOUNCE) && !state)
			{
				changed = 1;
				state = 0x80;
			}

			count++;
		}
		else
		{
			return;
		}
	}
	else
	{
		if (count > 0)
		{
			if ((count == 1) && state)
			{
				changed = 1;
				state = 0;
			}

			count--;
		}
		else
		{
			return;
		}
	}

	s_state[index] = state | count;

	if (changed)
	{
		if ((int8_t)index == s_shift_idx)
			s_shift_cleanup = 1;
		else
			SetNeedUpdate(index);
	}
}

static void ScanInputs(void)
{
	if (s_shift_cleanup)
		return;

	uint32_t mask = 0;

	for (uint8_t i = 0; i < s_nin; i++)
	{
		input_t const * const in = &s_in[i];
		uint8_t const level = (*PIN_REG_PIN(&in->io) & in->io.mask) ? 1 : 0;
		uint8_t const pressed = in->active_high ? level : (uint8_t)!level;

		SetInputCount(i, pressed);

		if (IsKeyDown(i))
			mask |= (uint32_t)1 << i;
	}

	s_input_mask = mask;

	CheckEncoder(s_mx_clk, s_mx_dir, &s_mx_last_clk, &s_mx_last_dir, &s_mouse_x);
	CheckEncoder(s_my_clk, s_my_dir, &s_my_last_clk, &s_my_last_dir, &s_mouse_y);
}

uint32_t panel_get_input_mask(void)
{
	return s_input_mask;
}

static uint8_t NeedUpdate(void)
{
	ShiftKeyCleanUp();

	if (s_need_mouse)
	{
		s_need_mouse = 0;
		return ID_Mouse;
	}

	if (s_need_key)
	{
		s_need_key = 0;
		return ID_Keyboard;
	}

	if (s_need_consumer)
	{
		s_need_consumer = 0;
		return ID_Consumer;
	}

	/* Joysticks der Reihe nach / joysticks round robin */
	for (uint8_t n = 0; n < NUM_JOYSTICKS; n++)
	{
		uint8_t const j = (uint8_t)((s_last_joy + 1 + n) % NUM_JOYSTICKS);

		if (s_need_joy[j])
		{
			s_need_joy[j] = 0;
			s_last_joy = j;
			return (uint8_t)(ID_Joystick1 + j);
		}
	}

	return ID_Unknown;
}


/* ---------------------------------------------------------------- Reports */

static uint8_t ReportConsumer(void)
{
	uint8_t consumer = 0;

	for (uint8_t i = 0; i < s_nin; i++)
	{
		if (IsKeyDown(i))
		{
			uint8_t const key = GetKey(i);

			if (IsConsumerCode(key))
				consumer |= ConsumerBit(key);
		}
	}

	s_report[0] = ID_Consumer;
	s_report[1] = consumer;

	return CONSUMER_REPORT_SIZE;
}

static uint8_t ReportKeyboard(void)
{
	uint8_t r = 2;

	memset(&s_report[0], 0x00, sizeof(s_report));
	s_report[0] = ID_Keyboard;

	for (uint8_t i = 0; i < s_nin; i++)
	{
		if (!IsKeyDown(i))
			continue;

		uint8_t const key = GetKey(i);

		if (!IsKeyboardCode(key))
			continue;

		if (IsModifierCode(key))
		{
			s_report[1] |= ModifierBit(key);
		}
		else if (r < KEYBOARD_REPORT_SIZE)
		{
			switch (key)
			{
			case KM_ALT_F4:
				s_report[1] |= ModifierBit(MOD_LeftAlt);
				s_report[r] = KEY_F4;
				break;
			case KM_SHIFT_F7:
				s_report[1] |= ModifierBit(MOD_LeftShift);
				s_report[r] = KEY_F7;
				break;
			default:
				s_report[r] = key;
				break;
			}

			r++;
		}
	}

	return KEYBOARD_REPORT_SIZE;
}

static uint8_t ReportJoystick(uint8_t id)
{
	uint8_t const joy = (uint8_t)(id - ID_Joystick1);
	int8_t x  = s_axis_val[joy][AXIS_X];
	int8_t y  = s_axis_val[joy][AXIS_Y];
	int8_t z  = s_axis_val[joy][AXIS_Z];
	int8_t rz = s_axis_val[joy][AXIS_RZ];
	uint8_t buttons = 0;

	uint8_t const base = (uint8_t)(joy * NR_OF_EVENTS_PER_JOY);

	for (uint8_t i = 0; i < s_nin; i++)
	{
		if (!IsKeyDown(i))
			continue;

		uint8_t const key = (uint8_t)(GetKey(i) - base);

		switch (key)
		{
		case J1_Left:  x = -127; break;
		case J1_Right: x = +127; break;
		case J1_Up:    y = -127; break;
		case J1_Down:  y = +127; break;

		case J1_Button1: case J1_Button2: case J1_Button3: case J1_Button4:
		case J1_Button5: case J1_Button6: case J1_Button7: case J1_Button8:
			buttons |= JoyButtonBit(key);
			break;

		default:
			break;
		}
	}

	for (uint8_t k = 0; k < NUM_AXES_PER_JOY; k++)
		s_axis_sent[joy][k] = s_axis_val[joy][k];

	s_report[0] = id;
	s_report[1] = (uint8_t)x;
	s_report[2] = (uint8_t)y;
	s_report[3] = (uint8_t)z;
	s_report[4] = (uint8_t)rz;
	s_report[5] = buttons;

	return JOYSTICK_REPORT_SIZE;
}

static uint8_t ReportMouse(void)
{
	uint8_t buttons = 0;

	for (uint8_t i = 0; i < s_nin; i++)
	{
		if (IsKeyDown(i))
		{
			uint8_t const key = GetKey(i);

			if (IsMouseButtonCode(key))
				buttons |= MouseButtonBit(key);
		}
	}

	s_report[0] = ID_Mouse;
	s_report[1] = buttons;
	s_report[2] = (uint8_t)s_mouse_x;
	s_report[3] = (uint8_t)s_mouse_y;
	s_mouse_x = 0;
	s_mouse_y = 0;

	return MOUSE_REPORT_SIZE;
}

static uint8_t BuildReport(uint8_t id)
{
	switch (id)
	{
	case ID_Keyboard:   return ReportKeyboard();
	case ID_Consumer:   return ReportConsumer();
	case ID_Mouse:      return ReportMouse();

	case ID_Joystick1:
	case ID_Joystick2:
	case ID_Joystick3:
	case ID_Joystick4:
		if ((uint8_t)(id - ID_Joystick1) < NUM_JOYSTICKS)
			return ReportJoystick(id);
		break;

	default:
		break;
	}

	return 0;
}

uint8_t panel_get_report(uint8_t **ppdata)
{
	if (ppdata == NULL)
		return 0;

	static uint16_t time_next_ms = 0;
	uint16_t const time_curr_ms = clock_ms();

	if ((int16_t)(time_curr_ms - time_next_ms) < 0)
		return 0;

	time_next_ms = time_curr_ms + SCAN_INTERVAL_MS;

	ScanInputs();
	UpdateAnalogAxes();

	uint8_t const id = NeedUpdate();

	if (id == ID_Unknown)
		return 0;

	uint8_t const ndata = BuildReport(id);
	*ppdata = s_report;

	return ndata;
}


#endif
