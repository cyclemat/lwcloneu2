/*
 * LWCloneU2 – WS2811/WS2812-Strip als eigenes Modul / WS2811/WS2812 strip as a separate module
 *
 * Direkt-Modus: der Host setzt Pixel per USB (Befehle 0xB0–0xB3, siehe proto.h) oder über den seriellen
 * Zugang (strip_serial.h). Spiegel-Modus: Pixelbereiche folgen den LED-Wiz-Ausgängen (outputs_get_level()).
 * Direct mode: the host sets pixels via USB (commands 0xB0–0xB3, see proto.h) or via the serial access
 * (strip_serial.h). Mirror mode: pixel ranges follow the LED-Wiz outputs (outputs_get_level()).
 */

#ifndef STRIP_H__INCLUDED
#define STRIP_H__INCLUDED

#include <stdint.h>
#include <stdbool.h>

/* kürzester Abstand zwischen zwei Ausgaben / minimum time between two refreshes */
#define STRIP_REFRESH_MS   10

/* Quellen einer Ausgabe-Anforderung; die Antwort geht nur an die anfragende Quelle
 * sources of a show request; the reply goes only to the requesting source */
#define STRIP_SHOW_HID     0x01
#define STRIP_SHOW_SERIAL  0x02

void strip_init(void);
void strip_apply_config(void);

/* regelmäßig aus der Hauptschleife / periodically from the main loop */
void strip_task(void);

/* USB-Befehle 0xB0..0xBF; true = reply8 sofort senden / true = send reply8 right away */
bool strip_handle_report(const uint8_t *p8, uint8_t *reply8);

/* Direkt-Zugriff für weitere Transportwege (serielles Protokoll) / direct access for other transports (serial protocol) */
bool    strip_direct_ready(void);                                        /* Strip aktiv und Direkt-Modus / active and direct mode */
uint8_t strip_count(void);                                               /* aktive Pixelanzahl / active pixel count */
bool    strip_set_count(uint8_t n);                                      /* zur Laufzeit, nicht gespeichert / runtime only, not saved */
void    strip_set_pixel(uint8_t idx, uint8_t r, uint8_t g, uint8_t b);
void    strip_fill(uint8_t first, uint8_t n, uint8_t r, uint8_t g, uint8_t b);
void    strip_clear(void);
void    strip_show(uint8_t source);                                      /* STRIP_SHOW_* */

#endif
