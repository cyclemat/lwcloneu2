/*
 * LWCloneU2 (Due) – USB-Gerät: drei HID-Schnittstellen (Misc, Panel, LED-Wiz) als PluggableUSB-Modul
 * neben dem CDC-Port des Arduino-Cores; Gerätedeskriptor mit LED-Wiz-VID/PID zur Laufzeit.
 *
 * LWCloneU2 (Due) – USB device: three HID interfaces (misc, panel, LED-Wiz) as a PluggableUSB module
 * next to the Arduino core's CDC port; device descriptor with LED-Wiz VID/PID at runtime.
 */

#ifndef USB_LWZ_H__INCLUDED
#define USB_LWZ_H__INCLUDED

#include <stdint.h>
#include <stdbool.h>

enum {
	LWZ_IF_MISC  = 0,
	LWZ_IF_PANEL = 1,
	LWZ_IF_LED   = 2,
};

#define LED_REPLY_SIZE  64     /* Input-Report der LED-Wiz-Schnittstelle / input report of the LED-Wiz interface */

/* LED-Wiz-ID 0..15 → PID 0x00F0 + id; muss vor dem USB-Start (initVariant) gesetzt sein
 * LED-Wiz id 0..15 → PID 0x00F0 + id; must be set before USB starts (initVariant) */
void lwz_usb_set_id(uint8_t ledwiz_id);

/* IN-Endpunkt frei? / IN endpoint free? */
bool lwz_usb_in_ready(uint8_t which);

/* Report senden (which = LWZ_IF_*); false wenn Endpunkt belegt oder nicht konfiguriert
 * send a report; false if the endpoint is busy or the device is not configured */
bool lwz_usb_send(uint8_t which, const uint8_t *data, uint8_t len);

/* Nächsten 8-Byte-Output-Report (SET_REPORT vom Host) holen / fetch the next 8 byte output report */
bool lwz_usb_pop_report(uint8_t *dst8);

/* CDC-Daten an den Host, ohne auf DTR zu achten (DOF setzt kein DTR); verwirft, wenn der Endpunkt belegt ist
 * CDC data to the host regardless of DTR (DOF does not assert DTR); drops if the endpoint is busy */
void lwz_cdc_send(const uint8_t *p, uint8_t n);

#endif
