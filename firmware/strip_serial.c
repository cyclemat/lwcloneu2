/*
 * LWCloneU2 – serieller LED-Strip-Zugang (DOF-TeensyStripController-Protokoll), siehe strip_serial.h
 * LWCloneU2 – serial LED strip access (DOF TeensyStripController protocol), see strip_serial.h
 *
 * Zustandsautomat statt blockierendem Warten: jedes Byte wird sofort verarbeitet, die Hauptschleife läuft weiter.
 * State machine instead of blocking waits: every byte is handled immediately, the main loop keeps running.
 */

#include <stdint.h>
#include <stdbool.h>

#include <hwconfig.h>
#include "config.h"
#include "proto.h"
#include "strip.h"
#include "strip_serial.h"


#if defined(ENABLE_STRIP_SERIAL) && (WS2812_MAX_PIXELS > 0)

enum {
	ST_CMD,      /* Befehlsbyte erwartet / waiting for command byte */
	ST_L,        /* 2 Byte Länge / 2 bytes length */
	ST_F,        /* 7 Byte: first(2) count(2) r g b */
	ST_R_HDR,    /* 4 Byte: first(2) count(2) */
	ST_R_DATA,   /* 3 Byte je Pixel / 3 bytes per pixel */
};

static uint8_t  s_state = ST_CMD;
static uint8_t  s_hdr[7];
static uint8_t  s_hlen;
static uint8_t  s_rgb[3];
static uint8_t  s_rlen;
static uint16_t s_pos;       /* nächster Pixel bei 'R' / next pixel for 'R' */
static uint16_t s_remain;    /* restliche Pixel bei 'R' / remaining pixels for 'R' */


static void ack(void)  { uint8_t const a = 'A'; strip_serial_tx(&a, 1); }
static void nack(void) { uint8_t const n = 'N'; strip_serial_tx(&n, 1); }

static inline uint16_t be16(const uint8_t *p) { return ((uint16_t)p[0] << 8) | p[1]; }


void strip_serial_show_done(void)
{
	ack();
}


void strip_serial_rx(uint8_t b)
{
	switch (s_state)
	{
	case ST_CMD:
		switch (b)
		{
		case 'L': s_state = ST_L;     s_hlen = 0; break;
		case 'F': s_state = ST_F;     s_hlen = 0; break;
		case 'R': s_state = ST_R_HDR; s_hlen = 0; break;

		case 'O':
			if (strip_direct_ready())
				strip_show(STRIP_SHOW_SERIAL);   /* 'A' folgt aus strip_task() / 'A' follows from strip_task() */
			else
				nack();
			break;

		case 'C':
			if (strip_direct_ready()) { strip_clear(); ack(); }
			else nack();
			break;

		case 'V':
		{
			uint8_t const v[3] = { FW_VERSION_MAJOR, FW_VERSION_MINOR, 'A' };
			strip_serial_tx(v, 3);
			break;
		}

		case 'M':
		{
			uint8_t const m[3] = { (uint8_t)(WS2812_MAX_PIXELS >> 8), (uint8_t)(WS2812_MAX_PIXELS & 0xFF), 'A' };
			strip_serial_tx(m, 3);
			break;
		}

		default:
			/* unbekannt – auch das 0x00, mit dem DOF beim Verbinden prüft, ob wir Befehle annehmen
			 * unknown – including the 0x00 DOF sends on connect to check we accept commands */
			nack();
			break;
		}
		break;

	case ST_L:
		s_hdr[s_hlen++] = b;
		if (s_hlen == 2)
		{
			uint16_t const n = be16(s_hdr);
			s_state = ST_CMD;

			if (n >= 1 && n <= WS2812_MAX_PIXELS && strip_direct_ready() && strip_set_count((uint8_t)n))
				ack();
			else
				nack();
		}
		break;

	case ST_F:
		s_hdr[s_hlen++] = b;
		if (s_hlen == 7)
		{
			uint16_t const first = be16(&s_hdr[0]);
			uint16_t const n     = be16(&s_hdr[2]);
			s_state = ST_CMD;

			if (strip_direct_ready() && n > 0 && first + n <= strip_count())
			{
				strip_fill((uint8_t)first, (uint8_t)n, s_hdr[4], s_hdr[5], s_hdr[6]);
				ack();
			}
			else
				nack();
		}
		break;

	case ST_R_HDR:
		s_hdr[s_hlen++] = b;
		if (s_hlen == 4)
		{
			uint16_t const first = be16(&s_hdr[0]);
			uint16_t const n     = be16(&s_hdr[2]);

			if (strip_direct_ready() && n > 0 && first + n <= strip_count())
			{
				s_pos = first;
				s_remain = n;
				s_rlen = 0;
				s_state = ST_R_DATA;
			}
			else
			{
				/* wie der Teensy: sofort 'N', die folgenden Datenbytes werden als Befehle gelesen (ergibt weitere 'N')
				 * like the Teensy: 'N' right away, the data bytes that follow are read as commands (more 'N') */
				s_state = ST_CMD;
				nack();
			}
		}
		break;

	case ST_R_DATA:
		s_rgb[s_rlen++] = b;
		if (s_rlen == 3)
		{
			s_rlen = 0;
			strip_set_pixel((uint8_t)s_pos, s_rgb[0], s_rgb[1], s_rgb[2]);
			s_pos++;

			if (--s_remain == 0)
			{
				s_state = ST_CMD;
				ack();
			}
		}
		break;

	default:
		s_state = ST_CMD;
		break;
	}
}

#else

void strip_serial_rx(uint8_t b) { (void)b; }
void strip_serial_show_done(void) {}

#endif
