/*
 * LWCloneU2 (Due) – USB-Gerät: drei HID-Schnittstellen (Misc, Panel, LED-Wiz) als PluggableUSB-Modul
 * LWCloneU2 (Due) – USB device: three HID interfaces (misc, panel, LED-Wiz) as a PluggableUSB module
 *
 * Copyright (C) 2026 cyclemat
 * Report-Deskriptoren nach descriptors.c (Andreas Dittrich 2013, LUFA-Demo Dean Camera 2012)
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
 * Der Arduino-SAM-Core fragt vor jedem Deskriptor zuerst die PluggableUSB-Module (USBD_SendDescriptor).
 * Deshalb kann dieses Modul den Gerätedeskriptor selbst liefern – mit VID 0xFAFA und der PID
 * 0x00F0 + LED-Wiz-ID aus dem Flash – ohne den Core zu ändern. Die Seriennummer baut der Core aus
 * getShortName(). Output-Reports (8 Byte, LED-Wiz-Befehle) kommen als SET_REPORT über Endpunkt 0 im
 * USB-Interrupt an und werden in eine Warteschlange gelegt; die Hauptschleife verarbeitet sie.
 *
 * The Arduino SAM core asks the PluggableUSB modules first for every descriptor (USBD_SendDescriptor).
 * So this module can deliver the device descriptor itself – with VID 0xFAFA and PID 0x00F0 + LED-Wiz
 * id from flash – without touching the core. The serial number is built by the core from
 * getShortName(). Output reports (8 bytes, LED-Wiz commands) arrive as SET_REPORT on endpoint 0 in
 * the USB interrupt and are queued; the main loop processes them.
 *
 * Reihenfolge / layout:  IF0/1 CDC (Core, EP1..3)   IF2 Misc (EP4)   IF3 Panel (EP5)   IF4 LED-Wiz (EP6)
 */

#include <Arduino.h>
#include <string.h>
#include "USB/USBAPI.h"
#include "USB/USBCore.h"
#include "USB/USBDesc.h"
#include "USB/PluggableUSB.h"
#include "lwc_shared.h"
#include "usb_lwz.h"


#define LWZ_USB_VID          0xFAFA
#define LWZ_USB_PID_BASE     0x00F0
#define LWZ_VERSION_BYTE     ((FW_VERSION_MAJOR << 4) | FW_VERSION_MINOR)   /* 0x21 */

#define USB_CLASS_HID                0x03
#define HID_DESCRIPTOR_TYPE          0x21
#define HID_REPORT_DESCRIPTOR_TYPE   0x22
#define HID_REQ_GET_REPORT   0x01
#define HID_REQ_GET_IDLE     0x02
#define HID_REQ_GET_PROTOCOL 0x03
#define HID_REQ_SET_REPORT   0x09
#define HID_REQ_SET_IDLE     0x0A
#define HID_REQ_SET_PROTOCOL 0x0B

/* bmRequestType */
#define RT_DEV_TO_HOST_STD_DEVICE     0x80
#define RT_DEV_TO_HOST_STD_INTERFACE  0x81
#define RT_DEV_TO_HOST_CLASS_IFACE    0xA1
#define RT_HOST_TO_DEV_CLASS_IFACE    0x21

/* Endpunktgrößen und Intervalle. Der Due läuft mit High-Speed: bInterval = 2^(n-1) × 125 µs,
 * n = 4 → 1 ms, n = 3 → 0,5 ms. Bei Full-Speed (alter Hub) sind das 4 bzw. 3 ms.
 * Endpoint sizes and intervals. The Due runs high speed: bInterval = 2^(n-1) × 125 µs. */
#define MISC_EPSIZE   64
#define PANEL_EPSIZE   8
#define LED_EPSIZE    64
#define MISC_INTERVAL  4
#define PANEL_INTERVAL 3
#define LED_INTERVAL   4

#define REPORT_QUEUE_LEN 16


/* ------------------------------------------------------------------ Report-Deskriptoren (wie descriptors.c) */

/* Misc: herstellerspezifisch, 64 Byte IN, 8 Byte OUT */
static const uint8_t MiscReport[] = {
	0x06, 0x00, 0xFF,       /* USAGE_PAGE (Vendor 0xFF00)   */
	0x09, 0x01,             /* USAGE (1)                    */
	0xA1, 0x01,             /* COLLECTION (Application)     */
	0x15, 0x00,             /*   LOGICAL_MINIMUM (0)        */
	0x25, 0xFF,             /*   LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,             /*   REPORT_SIZE (8)            */
	0x95, 0x40,             /*   REPORT_COUNT (64)          */
	0x09, 0x01,             /*   USAGE (1)                  */
	0x81, 0x02,             /*   INPUT (Data,Var,Abs)       */
	0x95, 0x08,             /*   REPORT_COUNT (8)           */
	0x09, 0x01,             /*   USAGE (1)                  */
	0x91, 0x02,             /*   OUTPUT (Data,Var,Abs)      */
	0xC0,                   /* END_COLLECTION               */
};

/* LED-Wiz: Output-Report 8 Byte (Befehle), Input-Report 64 Byte (Antworten, proto.h) */
static const uint8_t LEDReport[] = {
	0x06, 0x00, 0xFF,       /* USAGE_PAGE (Vendor 0xFF00)   */
	0x09, 0x01,             /* USAGE (1)                    */
	0xA1, 0x01,             /* COLLECTION (Application)     */
	0x15, 0x00,             /*   LOGICAL_MINIMUM (0)        */
	0x25, 0xFF,             /*   LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,             /*   REPORT_SIZE (8)            */
	0x95, 0x40,             /*   REPORT_COUNT (64)          */
	0x09, 0x02,             /*   USAGE (2)                  */
	0x81, 0x02,             /*   INPUT (Data,Var,Abs)       */
	0x95, 0x08,             /*   REPORT_COUNT (8)           */
	0x09, 0x03,             /*   USAGE (3)                  */
	0x91, 0x02,             /*   OUTPUT (Data,Var,Abs)      */
	0xC0,                   /* END_COLLECTION               */
};

#define JOYSTICK_DESCRIPTOR(id) \
	0x05, 0x01,  0x09, 0x05,  0xA1, 0x01,  0x85, id, \
	0x09, 0x01,  0xA1, 0x00, \
	0x09, 0x30,  0x09, 0x31,  0x09, 0x32,  0x09, 0x35, \
	0x15, 0x81,  0x25, 0x7F,  0x75, 0x08,  0x95, 0x04,  0x81, 0x02, \
	0xC0, \
	0x05, 0x09,  0x19, 0x01,  0x29, 0x08,  0x15, 0x00,  0x25, 0x01,  0x75, 0x01,  0x95, 0x08,  0x81, 0x02, \
	0xC0,

/* Panel: Tastatur (ID 1), Consumer (ID 2), 4 Gamepads (ID 3..6), Maus (ID 7) – identisch zur AVR-Firmware */
static const uint8_t PanelReport[] = {
	/* Tastatur / keyboard: Modifier + 6 Tasten */
	0x05, 0x01,  0x09, 0x06,  0xA1, 0x01,  0x85, ID_Keyboard,
	0x05, 0x07,  0x19, 0xE0,  0x29, 0xE7,  0x15, 0x00,  0x25, 0x01,  0x75, 0x01,  0x95, 0x08,  0x81, 0x02,
	0x95, 0x06,  0x75, 0x08,  0x25, 0x65,  0x19, 0x00,  0x29, 0x65,  0x81, 0x00,
	0xC0,
	/* Consumer: Lauter, Leiser, Stumm */
	0x05, 0x0C,  0x09, 0x01,  0xA1, 0x01,  0x85, ID_Consumer,
	0x15, 0x00,  0x25, 0x01,  0x09, 0xE9,  0x09, 0xEA,  0x75, 0x01,  0x95, 0x02,  0x81, 0x02,
	0x09, 0xE2,  0x95, 0x01,  0x81, 0x06,  0x95, 0x05,  0x81, 0x03,
	0xC0,
	JOYSTICK_DESCRIPTOR(ID_Joystick1)
	JOYSTICK_DESCRIPTOR(ID_Joystick2)
	JOYSTICK_DESCRIPTOR(ID_Joystick3)
	JOYSTICK_DESCRIPTOR(ID_Joystick4)
	/* Maus / mouse: 3 Tasten, X/Y relativ */
	0x05, 0x01,  0x09, 0x02,  0xA1, 0x01,  0x09, 0x01,  0xA1, 0x00,  0x85, ID_Mouse,
	0x05, 0x09,  0x19, 0x01,  0x29, 0x03,  0x15, 0x00,  0x25, 0x01,  0x95, 0x03,  0x75, 0x01,  0x81, 0x02,
	0x95, 0x01,  0x75, 0x05,  0x81, 0x03,
	0x05, 0x01,  0x09, 0x30,  0x09, 0x31,  0x15, 0x81,  0x25, 0x7F,  0x75, 0x08,  0x95, 0x02,  0x81, 0x06,
	0xC0,  0xC0,
};


/* ------------------------------------------------------------------ Deskriptor-Strukturen */

typedef struct __attribute__((packed)) {
	uint8_t  len;           /* 9 */
	uint8_t  dtype;         /* 0x21 */
	uint16_t bcdHID;        /* 0x0111 */
	uint8_t  country;
	uint8_t  numDescriptors;
	uint8_t  reportType;    /* 0x22 */
	uint16_t reportLength;
} HIDDescDescriptor;

typedef struct __attribute__((packed)) {
	InterfaceDescriptor itf;
	HIDDescDescriptor   hid;
	EndpointDescriptor  ep;
} HIDInterface;

typedef struct __attribute__((packed)) {
	HIDInterface misc;
	HIDInterface panel;
	HIDInterface led;
} LwzConfigPart;

#define D_HIDREPORT(length) { 9, HID_DESCRIPTOR_TYPE, 0x0111, 0, 1, HID_REPORT_DESCRIPTOR_TYPE, (length) }


/* ------------------------------------------------------------------ Modul */

class LwzUSB_ : public PluggableUSBModule
{
public:
	LwzUSB_();

	bool inReady(uint8_t which) const;
	bool send(uint8_t which, const uint8_t *data, uint8_t len);
	bool popReport(uint8_t *dst8);
	void setId(uint8_t id) { ledwizId = id & 0x0F; }

protected:
	int  getInterface(uint8_t *interfaceCount) override;
	int  getDescriptor(USBSetup &setup) override;
	bool setup(USBSetup &setup) override;
	uint8_t getShortName(char *name) override;

private:
	uint32_t epTypes[3];
	uint8_t  ledwizId;

	volatile uint8_t qHead, qTail;
	uint8_t  queue[REPORT_QUEUE_LEN][PROTO_REPORT_SIZE];

	void pushReport(const uint8_t *p8);
	uint16_t flags() const;
};

static LwzUSB_ s_lwz;


LwzUSB_::LwzUSB_() : PluggableUSBModule(3, 3, epTypes), ledwizId(0), qHead(0), qTail(0)
{
	epTypes[0] = EP_TYPE_INTERRUPT_IN;   /* Misc  */
	epTypes[1] = EP_TYPE_INTERRUPT_IN;   /* Panel */
	epTypes[2] = EP_TYPE_INTERRUPT_IN;   /* LED   */
	PluggableUSB().plug(this);
}

/* Schnittstellen-Flags wie descriptors.c (bcdDevice + Seriennummer gegen den Deskriptor-Cache von Windows)
 * interface flags like descriptors.c (bcdDevice + serial against Windows' descriptor cache) */
uint16_t LwzUSB_::flags() const
{
	return (NUM_JOYSTICKS & 7) | (1 << 3) /* keyboard */ | (1 << 4) /* mouse */ | (1 << 5) /* consumer */ |
	       (1 << 6) /* led */ | (1 << 7) /* serial */;
}

static uint16_t to_bcd(uint16_t n)
{
	uint16_t bcd = 0;
	for (uint8_t shift = 0; n != 0 && shift < 16; shift += 4, n /= 10)
		bcd |= (uint16_t)((n % 10) << shift);
	return bcd;
}

int LwzUSB_::getInterface(uint8_t *interfaceCount)
{
	uint8_t const ifn = pluggedInterface;
	uint8_t const ep  = pluggedEndpoint;

	LwzConfigPart const part = {
		{
			D_INTERFACE((uint8_t)(ifn + LWZ_IF_MISC), 1, USB_CLASS_HID, 0, 0),
			D_HIDREPORT(sizeof(MiscReport)),
			D_ENDPOINT(USB_ENDPOINT_IN(ep + LWZ_IF_MISC), USB_ENDPOINT_TYPE_INTERRUPT, MISC_EPSIZE, MISC_INTERVAL),
		},
		{
			D_INTERFACE((uint8_t)(ifn + LWZ_IF_PANEL), 1, USB_CLASS_HID, 0, 0),
			D_HIDREPORT(sizeof(PanelReport)),
			D_ENDPOINT(USB_ENDPOINT_IN(ep + LWZ_IF_PANEL), USB_ENDPOINT_TYPE_INTERRUPT, PANEL_EPSIZE, PANEL_INTERVAL),
		},
		{
			D_INTERFACE((uint8_t)(ifn + LWZ_IF_LED), 1, USB_CLASS_HID, 0, 0),
			D_HIDREPORT(sizeof(LEDReport)),
			D_ENDPOINT(USB_ENDPOINT_IN(ep + LWZ_IF_LED), USB_ENDPOINT_TYPE_INTERRUPT, LED_EPSIZE, LED_INTERVAL),
		},
	};

	*interfaceCount += 3;
	return USBD_SendControl(0, &part, sizeof(part));
}

int LwzUSB_::getDescriptor(USBSetup &setup)
{
	uint8_t const type = setup.wValueH;

	/* Gerätedeskriptor: LED-Wiz-VID/PID, zusammengesetztes Gerät (IAD wegen CDC) / composite device */
	if (setup.bmRequestType == RT_DEV_TO_HOST_STD_DEVICE && type == USB_DEVICE_DESCRIPTOR_TYPE)
	{
		uint16_t const pid = (uint16_t)(LWZ_USB_PID_BASE | ledwizId);
		uint16_t const rel = to_bcd((uint16_t)(((LWZ_VERSION_BYTE & 0x1F) << 8) | flags()));

		DeviceDescriptor const dev = D_DEVICE(0xEF, 0x02, 0x01, 64, LWZ_USB_VID, pid, rel,
		                                      IMANUFACTURER, IPRODUCT, ISERIAL, 1);
		return USBD_SendControl(0, &dev, sizeof(dev));
	}

	if (setup.bmRequestType != RT_DEV_TO_HOST_STD_INTERFACE)
		return 0;

	uint8_t const itf = (uint8_t)setup.wIndex;

	if (itf < pluggedInterface || itf >= pluggedInterface + 3)
		return 0;

	uint8_t const which = (uint8_t)(itf - pluggedInterface);
	const uint8_t *report; uint16_t len;

	switch (which)
	{
	case LWZ_IF_MISC:  report = MiscReport;  len = sizeof(MiscReport);  break;
	case LWZ_IF_PANEL: report = PanelReport; len = sizeof(PanelReport); break;
	default:           report = LEDReport;   len = sizeof(LEDReport);   break;
	}

	if (type == HID_REPORT_DESCRIPTOR_TYPE)
		return USBD_SendControl(0, report, len);

	if (type == HID_DESCRIPTOR_TYPE)
	{
		HIDDescDescriptor const hid = D_HIDREPORT(len);
		return USBD_SendControl(0, &hid, sizeof(hid));
	}

	return 0;
}

bool LwzUSB_::setup(USBSetup &setup)
{
	uint8_t const itf = (uint8_t)setup.wIndex;

	if (itf < pluggedInterface || itf >= pluggedInterface + 3)
		return false;

	uint8_t const rt = setup.bmRequestType;
	uint8_t const r  = setup.bRequest;

	if (rt == RT_DEV_TO_HOST_CLASS_IFACE)
	{
		if (r == HID_REQ_GET_REPORT || r == HID_REQ_GET_IDLE)
		{
			uint8_t const zero = 0;              /* wie die AVR-Firmware: ein Nullbyte / like the AVR firmware */
			USBD_SendControl(0, &zero, 1);
			return true;
		}
		if (r == HID_REQ_GET_PROTOCOL)
		{
			uint8_t const report_protocol = 1;
			USBD_SendControl(0, &report_protocol, 1);
			return true;
		}
		return false;
	}

	if (rt == RT_HOST_TO_DEV_CLASS_IFACE)
	{
		if (r == HID_REQ_SET_REPORT)
		{
			uint8_t buf[64];
			uint16_t n = setup.wLength;

			if (n > sizeof(buf))
				n = sizeof(buf);

			memset(buf, 0x00, sizeof(buf));

			if (n > 0)
				USBD_RecvControl(buf, n);

			/* Output-Reports jeder unserer Schnittstellen sind LED-Wiz-Befehle (wie main_usb.c)
			 * output reports on any of our interfaces are LED-Wiz commands (like main_usb.c) */
			pushReport(buf);
			return true;
		}
		if (r == HID_REQ_SET_IDLE || r == HID_REQ_SET_PROTOCOL)
			return true;
	}

	return false;
}

/* Seriennummer "LWC-00vv-ii-fff" wie descriptors.c (vv Version, ii LED-Wiz-ID-1 hex, fff Flags) */
uint8_t LwzUSB_::getShortName(char *name)
{
	static const char hex[] = "0123456789ABCDEF";
	uint16_t const f = flags();
	uint8_t const ver = LWZ_VERSION_BYTE;
	uint8_t const id = ledwizId;        /* PID-Nibble = LED-Wiz-ID − 1 / PID nibble = LED-Wiz id − 1 */

	const char s[15] = {
		'L', 'W', 'C', '-',
		'0', '0', (char)('0' + ((ver >> 4) & 0x0F)), (char)('0' + (ver & 0x0F)), '-',
		hex[(id >> 4) & 0x0F], hex[id & 0x0F], '-',
		hex[(f >> 8) & 0x0F], hex[(f >> 4) & 0x0F], hex[f & 0x0F],
	};
	memcpy(name, s, sizeof(s));
	return sizeof(s);
}

void LwzUSB_::pushReport(const uint8_t *p8)
{
	uint8_t const next = (uint8_t)((qHead + 1) % REPORT_QUEUE_LEN);

	if (next == qTail)
		return;                       /* voll: verwerfen / full: drop */

	memcpy(queue[qHead], p8, PROTO_REPORT_SIZE);
	qHead = next;
}

bool LwzUSB_::popReport(uint8_t *dst8)
{
	bool res = false;

	uint32_t const primask = __get_PRIMASK();
	__disable_irq();

	if (qTail != qHead)
	{
		memcpy(dst8, queue[qTail], PROTO_REPORT_SIZE);
		qTail = (uint8_t)((qTail + 1) % REPORT_QUEUE_LEN);
		res = true;
	}

	__set_PRIMASK(primask);
	return res;
}

bool LwzUSB_::inReady(uint8_t which) const
{
	if (!USBDevice.configured())
		return false;

	uint8_t const ep = (uint8_t)(pluggedEndpoint + which);
	return (UOTGHS->UOTGHS_DEVEPTISR[ep] & UOTGHS_DEVEPTISR_TXINI) != 0;   /* eine Bank frei / a bank is free */
}

bool LwzUSB_::send(uint8_t which, const uint8_t *data, uint8_t len)
{
	if (!inReady(which))
		return false;

	USBD_Send(pluggedEndpoint + which, data, len);
	return true;
}


/* ------------------------------------------------------------------ C-Schnittstelle / C interface */

void lwz_usb_set_id(uint8_t ledwiz_id)              { s_lwz.setId(ledwiz_id); }
bool lwz_usb_in_ready(uint8_t which)                { return s_lwz.inReady(which); }
bool lwz_usb_send(uint8_t which, const uint8_t *data, uint8_t len) { return s_lwz.send(which, data, len); }
bool lwz_usb_pop_report(uint8_t *dst8)              { return s_lwz.popReport(dst8); }

void lwz_cdc_send(const uint8_t *p, uint8_t n)
{
	if (!USBDevice.configured())
		return;

	if ((UOTGHS->UOTGHS_DEVEPTISR[CDC_TX] & UOTGHS_DEVEPTISR_TXINI) == 0)
		return;                       /* Host holt nicht ab: verwerfen statt blockieren / host not reading: drop */

	USBD_Send(CDC_TX, p, n);
}
