/*
 * LWCloneU2 – serieller LED-Strip-Zugang (virtueller COM-Port, Protokoll des DOF TeensyStripController)
 * LWCloneU2 – serial LED strip access (virtual COM port, protocol of the DOF TeensyStripController)
 *
 * Damit kann das DirectOutput Framework (DOF) den Strip direkt ansteuern: in der Cabinet.xml ein
 * <TeensyStripController> mit dem COM-Port des LWCloneU2 und NumberOfLedsStrip1. Nur ein Kanal (Strip 1).
 * Lets the DirectOutput Framework (DOF) drive the strip directly: a <TeensyStripController> in Cabinet.xml
 * with the LWCloneU2's COM port and NumberOfLedsStrip1. One channel only (strip 1).
 *
 * Protokoll (Byte-Befehle, Zahlen Big-Endian, Antwort 'A' = ok, 'N' = Fehler) / protocol (big-endian, 'A' ok, 'N' error):
 *   'L' hi lo                 Pixelanzahl setzen (1..WS2812_MAX_PIXELS) / set pixel count
 *   'M'                       -> hi lo (max. Pixel) 'A'                 / max pixels
 *   'V'                       -> major minor 'A'                        / firmware version
 *   'C'                       alle Pixel schwarz / clear all pixels
 *   'O'                       ausgeben; 'A' kommt erst nach dem Schreiben des Strips / show; 'A' after the strip is written
 *   'R' hi lo hi lo + 3*n     n Pixel ab Index setzen (r g b je Pixel, Reihenfolge wird nicht geändert)
 *                             set n pixels from index (r g b each, order passed through)
 *   'F' hi lo hi lo r g b     n Pixel ab Index mit einer Farbe füllen / fill n pixels from index
 *   sonst / else              'N' (auch das 0x00 des DOF-Handshakes / also DOF's 0x00 handshake byte)
 *
 * Befehle außer 'M'/'V' brauchen einen eingerichteten Strip im Direkt-Modus, sonst 'N'.
 * Commands other than 'M'/'V' need a configured strip in direct mode, else 'N'.
 */

#ifndef STRIP_SERIAL_H__INCLUDED
#define STRIP_SERIAL_H__INCLUDED

#include <stdint.h>

/* Byte vom Host verarbeiten / process one byte from the host */
void strip_serial_rx(uint8_t b);

/* vom Strip-Modul nach dem Schreiben gerufen (Antwort auf 'O') / called by the strip module after writing (reply to 'O') */
void strip_serial_show_done(void);

/* Ausgabe zum Host – implementiert in main_usb.c (CDC) / output to the host – implemented in main_usb.c (CDC) */
void strip_serial_tx(const uint8_t *p, uint8_t n);

#endif
