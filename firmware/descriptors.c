/*
 * LWCloneU2
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
             LUFA Library
     Copyright (C) Dean Camera, 2012.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "descriptors.h"
#include "panel.h"

#if !defined(NUM_JOYSTICKS)
#define NUM_JOYSTICKS 4
#endif

#if (NUM_JOYSTICKS < 1) || (NUM_JOYSTICKS > 4)
#error "NUM_JOYSTICKS muss 1..4 sein / must be 1..4"
#endif


/* ------------------------------------------------------------------ Strings */

#define USB_STRING_TABLE(_map_) \
	_map_(ManufacturerString_id,  "LWCloneU2") \
	_map_(ProductString_id,       "LWCloneU2 v2")

typedef enum {
	LanguageString_id = 0,
	#define MAP(x, y) x,
	USB_STRING_TABLE(MAP)
	#undef MAP
	SerialNumberString_id,
} UsbStringsEnum;

static const USB_Descriptor_String_t PROGMEM LanguageString =
{
	.Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},
	.UnicodeString          = {LANGUAGE_ID_ENG}
};

#define MAP(name, str) \
static const USB_Descriptor_String_t PROGMEM name##__str_ = \
{ \
	.Header                 = {.Size = USB_STRING_LEN(sizeof(str) - 1), .Type = DTYPE_String}, \
	.UnicodeString          = L##str \
};
USB_STRING_TABLE(MAP)
#undef MAP


/* ------------------------------------------------------------------ Misc-Schnittstelle (herstellerspezifisch) */

static const USB_Descriptor_HIDReport_Datatype_t PROGMEM MiscReport[] =
{
	HID_RI_USAGE_PAGE(16, 0xFF00), /* Vendor Page 0 */
	HID_RI_USAGE(8, 0x01), /* Vendor Usage 1 */
	HID_RI_COLLECTION(8, 0x01), /* Vendor Usage 1 */
		HID_RI_LOGICAL_MINIMUM(8, 0x00),
		HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
		HID_RI_REPORT_SIZE(8, 0x08),
		HID_RI_REPORT_COUNT(8, 64),
		HID_RI_USAGE(8, 0x01), /* Vendor Usage 1 */
		HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
		HID_RI_REPORT_COUNT(8, 8),
		HID_RI_USAGE(8, 0x01), /* Vendor Usage 1 */
		HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),
	HID_RI_END_COLLECTION(0),
};


/* ------------------------------------------------------------------ Panel: Tastatur, Consumer, Joysticks, Maus */

#if defined(ENABLE_PANEL_DEVICE)

enum HID_KeyModifierCodes
{
	KEY_LeftControl      = 0xE0,
	KEY_RightGUI         = 0xE7,
};

enum HID_ConsumerCodes
{
	CKEY_Mute       = 0xE2,
	CKEY_VolumeUp   = 0xE9,
	CKEY_VolumeDown = 0xEA,
};

/* Joystick: 4 Achsen int8 (X, Y, Z, Rz) + 8 Tasten → 6 Byte inkl. Report-ID (panel.h: JOYSTICK_REPORT_SIZE) */
#define JOYSTICK_DESCRIPTOR(id) \
	0x05, 0x01,             /* USAGE_PAGE (Generic Desktop)   */ \
	0x09, 0x05,             /* USAGE (Gamepad)                */ \
	0xa1, 0x01,             /* COLLECTION (Application)       */ \
	0x85, id,               /*   REPORT_ID                    */ \
	0x09, 0x01,             /*   USAGE (Pointer)              */ \
	0xa1, 0x00,             /*   COLLECTION (Physical)        */ \
	0x09, 0x30,             /*     USAGE (X)                  */ \
	0x09, 0x31,             /*     USAGE (Y)                  */ \
	0x09, 0x32,             /*     USAGE (Z)                  */ \
	0x09, 0x35,             /*     USAGE (Rz)                 */ \
	0x15, 0x81,             /*     LOGICAL_MINIMUM (-127)     */ \
	0x25, 0x7F,             /*     LOGICAL_MAXIMUM (127)      */ \
	0x75, 0x08,             /*     REPORT_SIZE (8)            */ \
	0x95, 0x04,             /*     REPORT_COUNT (4)           */ \
	0x81, 0x02,             /*     INPUT (Data,Var,Abs)       */ \
	0xc0,                   /*   END_COLLECTION               */ \
	0x05, 0x09,             /*   USAGE_PAGE (Button)          */ \
	0x19, 0x01,             /*   USAGE_MINIMUM (Button 1)     */ \
	0x29, 0x08,             /*   USAGE_MAXIMUM (Button 8)     */ \
	0x15, 0x00,             /*   LOGICAL_MINIMUM (0)          */ \
	0x25, 0x01,             /*   LOGICAL_MAXIMUM (1)          */ \
	0x75, 0x01,             /*   REPORT_SIZE (1)              */ \
	0x95, 0x08,             /*   REPORT_COUNT (8)             */ \
	0x81, 0x02,             /*   INPUT (Data,Var,Abs)         */ \
	0xc0,                   /* END_COLLECTION                 */

static const USB_Descriptor_HIDReport_Datatype_t PROGMEM PanelReport[] =
{
	/* Tastatur: Modifier + 6 Tasten → 8 Byte / keyboard */
	0x05, 0x01,             // USAGE_PAGE (Generic Desktop)
	0x09, 0x06,             // USAGE (Keyboard)
	0xa1, 0x01,             // COLLECTION (Application)
	0x85, ID_Keyboard,      //   REPORT_ID (ID_Keyboard)
	0x05, 0x07,             //   USAGE_PAGE (Keyboard)
	0x19, KEY_LeftControl,  //   USAGE_MINIMUM (Keyboard LeftControl)
	0x29, KEY_RightGUI,     //   USAGE_MAXIMUM (Keyboard RightGUI)
	0x15, 0x00,             //   LOGICAL_MINIMUM (0)
	0x25, 0x01,             //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,             //   REPORT_SIZE (1)
	0x95, 0x08,             //   REPORT_COUNT (8)
	0x81, 0x02,             //   INPUT (Data,Var,Abs)
	0x95, 0x06,             //   REPORT_COUNT (6)
	0x75, 0x08,             //   REPORT_SIZE (8)
	0x25, 0x65,             //   LOGICAL_MAXIMUM (101)
	0x19, 0x00,             //   USAGE_MINIMUM (Reserved (no event indicated))
	0x29, 0x65,             //   USAGE_MAXIMUM (101)
	0x81, 0x00,             //   INPUT (Data,Ary,Abs)
	0xc0,                   // END_COLLECTION

	/* Consumer: Lauter, Leiser, Stumm → 2 Byte / consumer control */
	0x05, 0x0C,             // USAGE_PAGE (Consumer Devices)
	0x09, 0x01,             // USAGE (Consumer Control)
	0xa1, 0x01,             // COLLECTION (Application)
	0x85, ID_Consumer,      //   REPORT_ID (ID_Consumer)
	0x15, 0x00,             //   LOGICAL_MINIMUM (0)
	0x25, 0x01,             //   LOGICAL_MAXIMUM (1)
	0x09, CKEY_VolumeUp,    //   USAGE (Volume Up)
	0x09, CKEY_VolumeDown,  //   USAGE (Volume Down)
	0x75, 0x01,             //   REPORT_SIZE (1)
	0x95, 0x02,             //   REPORT_COUNT (2)
	0x81, 0x02,             //   INPUT (Data,Var,Abs)
	0x09, CKEY_Mute,        //   USAGE (Mute)
	0x95, 0x01,             //   REPORT_COUNT (1)
	0x81, 0x06,             //   INPUT (Data,Var,Rel)
	0x95, 0x05,             //   REPORT_COUNT (5)
	0x81, 0x03,             //   INPUT (Cnst,Var,Abs)
	0xc0,                   // END_COLLECTION

	JOYSTICK_DESCRIPTOR(ID_Joystick1)
	#if (NUM_JOYSTICKS >= 2)
	JOYSTICK_DESCRIPTOR(ID_Joystick2)
	#endif
	#if (NUM_JOYSTICKS >= 3)
	JOYSTICK_DESCRIPTOR(ID_Joystick3)
	#endif
	#if (NUM_JOYSTICKS >= 4)
	JOYSTICK_DESCRIPTOR(ID_Joystick4)
	#endif

	/* Maus: 3 Tasten, X/Y relativ → 4 Byte / mouse */
	0x05, 0x01,             // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,             // USAGE (Mouse)
	0xa1, 0x01,             // COLLECTION (Application)
	0x09, 0x01,             //   USAGE (Pointer)
	0xa1, 0x00,             //   COLLECTION (Physical)
	0x85, ID_Mouse,         //     REPORT_ID (ID_Mouse)
	0x05, 0x09,             //     USAGE_PAGE (Button)
	0x19, 0x01,             //     USAGE_MINIMUM (Button 1)
	0x29, 0x03,             //     USAGE_MAXIMUM (Button 3)
	0x15, 0x00,             //     LOGICAL_MINIMUM (0)
	0x25, 0x01,             //     LOGICAL_MAXIMUM (1)
	0x95, 0x03,             //     REPORT_COUNT (3)
	0x75, 0x01,             //     REPORT_SIZE (1)
	0x81, 0x02,             //     INPUT (Data,Var,Abs)
	0x95, 0x01,             //     REPORT_COUNT (1)
	0x75, 0x05,             //     REPORT_SIZE (5)
	0x81, 0x03,             //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,             //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,             //     USAGE (X)
	0x09, 0x31,             //     USAGE (Y)
	0x15, 0x81,             //     LOGICAL_MINIMUM (-127)
	0x25, 0x7f,             //     LOGICAL_MAXIMUM (127)
	0x75, 0x08,             //     REPORT_SIZE (8)
	0x95, 0x02,             //     REPORT_COUNT (2)
	0x81, 0x06,             //     INPUT (Data,Var,Rel)
	0xc0,                   //   END_COLLECTION
	0xc0,                   // END_COLLECTION
};

#endif


/* ------------------------------------------------------------------ LED-Wiz-Schnittstelle */

#if defined(ENABLE_LED_DEVICE)

#define LED_REPORT_SIZE 8

/* Output-Report 8 Byte (wie LED-Wiz), Input-Report 64 Byte für Antworten (proto.h) */
static const USB_Descriptor_HIDReport_Datatype_t PROGMEM LEDReport[] =
{
	HID_RI_USAGE_PAGE(16, 0xFF00), /* Vendor Page 0 */
	HID_RI_USAGE(8, 0x01), /* Vendor Usage 1 */
	HID_RI_COLLECTION(8, 0x01), /* Vendor Usage 1 */
		HID_RI_LOGICAL_MINIMUM(8, 0x00),
		HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
		HID_RI_REPORT_SIZE(8, 0x08),
		HID_RI_REPORT_COUNT(8, 64),
		HID_RI_USAGE(8, 0x02), /* Vendor Usage 2 */
		HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
		HID_RI_REPORT_COUNT(8, LED_REPORT_SIZE),
		HID_RI_USAGE(8, 0x03), /* Vendor Usage 3 */
		HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),
	HID_RI_END_COLLECTION(0),
};

#endif


/* ------------------------------------------------------------------ Gerätedeskriptor (RAM, PID wird gesetzt) */

static USB_Descriptor_Device_t DeviceDescriptor =
{
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},
	.USBSpecification       = XYZ_TO_BCD(1,1,0),
	#if defined(ENABLE_STRIP_SERIAL)
	/* zusammengesetztes Gerät mit Interface Association (CDC braucht das unter Windows)
	 * composite device with interface association (CDC needs this on Windows) */
	.Class                  = USB_CSCP_IADDeviceClass,
	.SubClass               = USB_CSCP_IADDeviceSubclass,
	.Protocol               = USB_CSCP_IADDeviceProtocol,
	#else
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,
	#endif
	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
	.VendorID               = USB_VENDOR_ID,
	.ProductID              = USB_PRODUCT_ID,
	.ReleaseNumber          = 0,
	.ManufacturerStrIndex   = ManufacturerString_id,
	.ProductStrIndex        = ProductString_id,
	.SerialNumStrIndex      = SerialNumberString_id,
	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

static char toHex(uint8_t x)
{
	if (x < 10)
		return '0' + x;

	return x + 'A' - 10;
}

/* Windows merkt sich Deskriptoren je VID/PID/Version. Damit unterschiedliche Builds nicht
 * mit einem alten Zwischenspeicher kollidieren, kodieren wir die Schnittstellen in der
 * Versionsnummer und in der Seriennummer.
 * Windows caches descriptors per VID/PID/release. To keep different builds from colliding
 * with a stale cache, the interfaces are encoded in the release number and serial string. */
typedef enum {
	config_mask_joysticks  = 7 << 0,
	config_flag_keyboard   = 1 << 3,
	config_flag_mouse      = 1 << 4,
	config_flag_consumer   = 1 << 5,
	config_flag_led        = 1 << 6,
	config_flag_serial     = 1 << 7,
} usb_config_flags;

static struct {
	USB_Descriptor_Header_t Header;
	uint16_t UnicodeString[15];
} SerialNumberString = {
	.Header = {
		.Size = USB_STRING_LEN(15),
		.Type = DTYPE_String
	}
};

void SetProductID(uint16_t id)
{
	uint8_t const ledwiz_id_minus1 = id & 0xFF;
	uint16_t const flags =
		#if defined(ENABLE_PANEL_DEVICE)
		(config_mask_joysticks & NUM_JOYSTICKS) | config_flag_keyboard | config_flag_mouse | config_flag_consumer |
		#endif
		#if defined(ENABLE_LED_DEVICE)
		config_flag_led |
		#endif
		#if defined(ENABLE_STRIP_SERIAL)
		config_flag_serial |
		#endif
		0;
	uint8_t const ver = LWCLONEU2_VERSION;
	uint16_t const rel = ((uint16_t)(ver & 0x1F) << 8) | flags; // maximal 13 Bit (BCD-Grenze 9999)

	DeviceDescriptor.ProductID = id;
	DeviceDescriptor.ReleaseNumber = NUMBER_TO_BCD(rel);

	uint16_t * const c = &SerialNumberString.UnicodeString[0];
	c[0]  = 'L';
	c[1]  = 'W';
	c[2]  = 'C';
	c[3]  = '-';
	c[4]  = '0';
	c[5]  = '0';
	c[6]  = '0' + ((ver >>  4) & 0x0F);
	c[7]  = '0' + ((ver >>  0) & 0x0F);
	c[8]  = '-';
	c[9]  = toHex((ledwiz_id_minus1 >> 4) & 0x0F);
	c[10] = toHex((ledwiz_id_minus1 >> 0) & 0x0F);
	c[11] = '-';
	c[12] = toHex((flags >> 8) & 0x0F);
	c[13] = toHex((flags >> 4) & 0x0F);
	c[14] = toHex((flags >> 0) & 0x0F);
}


/* ------------------------------------------------------------------ Konfigurationsdeskriptor */

static const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
	.Config =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},
		.TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
		.TotalInterfaces        = NUM_TOTAL_INTERFACES,
		.ConfigurationNumber    = 1,
		.ConfigurationStrIndex  = NO_DESCRIPTOR,
		.ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),
		.MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
	},

	.HID_MiscInterface =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
		.InterfaceNumber        = IFACENUMBER_MISC,
		.AlternateSetting       = 0x00,
		.TotalEndpoints         = 1,
		.Class                  = HID_CSCP_HIDClass,
		.SubClass               = HID_CSCP_NonBootSubclass,
		.Protocol               = HID_CSCP_NonBootProtocol,
		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.HID_MiscHID =
	{
		.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
		.HIDSpec                = XYZ_TO_BCD(1,1,1),
		.CountryCode            = 0x00,
		.TotalReportDescriptors = 1,
		.HIDReportType          = HID_DTYPE_Report,
		.HIDReportLength        = sizeof(MiscReport)
	},

	.HID_MiscReportINEndpoint =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = MISC_EPADDR,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = MISC_EPSIZE,
		.PollingIntervalMS      = MISC_INTERVAL_MS
	},

	#if defined(ENABLE_PANEL_DEVICE)
	.HID_PanelInterface =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
		.InterfaceNumber        = IFACENUMBER_PANEL,
		.AlternateSetting       = 0x00,
		.TotalEndpoints         = 1,
		.Class                  = HID_CSCP_HIDClass,
		.SubClass               = HID_CSCP_NonBootSubclass,
		.Protocol               = HID_CSCP_NonBootProtocol,
		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.HID_PanelHID =
	{
		.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
		.HIDSpec                = XYZ_TO_BCD(1,1,1),
		.CountryCode            = 0x00,
		.TotalReportDescriptors = 1,
		.HIDReportType          = HID_DTYPE_Report,
		.HIDReportLength        = sizeof(PanelReport)
	},

	.HID_PanelReportINEndpoint =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = PANEL_EPADDR,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = PANEL_EPSIZE,
		.PollingIntervalMS      = PANEL_INTERVAL_MS
	},
	#endif

	#if defined(ENABLE_LED_DEVICE)
	.HID_LEDInterface =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
		.InterfaceNumber        = IFACENUMBER_LED,
		.AlternateSetting       = 0x00,
		.TotalEndpoints         = 1,
		.Class                  = HID_CSCP_HIDClass,
		.SubClass               = HID_CSCP_NonBootSubclass,
		.Protocol               = HID_CSCP_NonBootProtocol,
		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.HID_LEDHID =
	{
		.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
		.HIDSpec                = XYZ_TO_BCD(1,1,1),
		.CountryCode            = 0x00,
		.TotalReportDescriptors = 1,
		.HIDReportType          = HID_DTYPE_Report,
		.HIDReportLength        = sizeof(LEDReport)
	},

	.HID_LEDReportINEndpoint =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = LED_EPADDR,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = LED_EPSIZE,
		.PollingIntervalMS      = LED_INTERVAL_MS
	},
	#endif

	#if defined(ENABLE_STRIP_SERIAL)
	/* CDC-ACM wie in den LUFA-Demos (VirtualSerial) / CDC ACM as in the LUFA demos */
	.CDC_IAD =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_Association_t), .Type = DTYPE_InterfaceAssociation},
		.FirstInterfaceIndex    = IFACENUMBER_CDC_CCI,
		.TotalInterfaces        = 2,
		.Class                  = CDC_CSCP_CDCClass,
		.SubClass               = CDC_CSCP_ACMSubclass,
		.Protocol               = CDC_CSCP_ATCommandProtocol,
		.IADStrIndex            = NO_DESCRIPTOR
	},

	.CDC_CCI_Interface =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
		.InterfaceNumber        = IFACENUMBER_CDC_CCI,
		.AlternateSetting       = 0,
		.TotalEndpoints         = 1,
		.Class                  = CDC_CSCP_CDCClass,
		.SubClass               = CDC_CSCP_ACMSubclass,
		.Protocol               = CDC_CSCP_ATCommandProtocol,
		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.CDC_Functional_Header =
	{
		.Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t), .Type = DTYPE_CSInterface},
		.Subtype                = CDC_DSUBTYPE_CSInterface_Header,
		.CDCSpecification       = XYZ_TO_BCD(1,1,0),
	},

	.CDC_Functional_ACM =
	{
		.Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t), .Type = DTYPE_CSInterface},
		.Subtype                = CDC_DSUBTYPE_CSInterface_ACM,
		.Capabilities           = 0x06,
	},

	.CDC_Functional_Union =
	{
		.Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t), .Type = DTYPE_CSInterface},
		.Subtype                = CDC_DSUBTYPE_CSInterface_Union,
		.MasterInterfaceNumber  = IFACENUMBER_CDC_CCI,
		.SlaveInterfaceNumber   = IFACENUMBER_CDC_DCI,
	},

	.CDC_NotificationEndpoint =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = CDC_NOTIFICATION_EPADDR,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = CDC_NOTIFICATION_EPSIZE,
		.PollingIntervalMS      = 0xFF
	},

	.CDC_DCI_Interface =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
		.InterfaceNumber        = IFACENUMBER_CDC_DCI,
		.AlternateSetting       = 0,
		.TotalEndpoints         = 2,
		.Class                  = CDC_CSCP_CDCDataClass,
		.SubClass               = CDC_CSCP_NoDataSubclass,
		.Protocol               = CDC_CSCP_NoDataProtocol,
		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.CDC_DataOutEndpoint =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = CDC_RX_EPADDR,
		.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = CDC_TXRX_EPSIZE,
		.PollingIntervalMS      = 0x05
	},

	.CDC_DataInEndpoint =
	{
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress        = CDC_TX_EPADDR,
		.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = CDC_TXRX_EPSIZE,
		.PollingIntervalMS      = 0x05
	},
	#endif
};


/* ------------------------------------------------------------------ Deskriptor-Abfrage */

uint16_t CALLBACK_USB_GetDescriptor(
	const uint16_t wValue,
	const uint8_t wIndex,
	const void** const DescriptorAddress,
	uint8_t *const DescriptorMemorySpace)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	uint8_t memspace;
	uint8_t * const pmemspace = (DescriptorMemorySpace != NULL) ? DescriptorMemorySpace : &memspace;

	*pmemspace = MEMSPACE_FLASH;

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			*pmemspace = MEMSPACE_RAM;
			Address = &DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;

		case DTYPE_Configuration:
			Address = &ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;

		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case LanguageString_id:
					Address = &LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;

				case SerialNumberString_id:
					*pmemspace = MEMSPACE_RAM;
					Address = &SerialNumberString;
					Size = SerialNumberString.Header.Size;
					break;

				#define MAP(name, str) \
				case name: \
					Address = &name##__str_; \
					Size    = pgm_read_byte(&name##__str_.Header.Size); \
					break;
				USB_STRING_TABLE(MAP)
				#undef MAP
			}
			break;

		case DTYPE_HID:
			if (wIndex == IFACENUMBER_MISC)
			{
				Address = &ConfigurationDescriptor.HID_MiscHID;
				Size    = sizeof(USB_HID_Descriptor_HID_t);
				break;
			}
			#if defined(ENABLE_LED_DEVICE)
			if (wIndex == IFACENUMBER_LED)
			{
				Address = &ConfigurationDescriptor.HID_LEDHID;
				Size    = sizeof(USB_HID_Descriptor_HID_t);
				break;
			}
			#endif
			#if defined(ENABLE_PANEL_DEVICE)
			if (wIndex == IFACENUMBER_PANEL)
			{
				Address = &ConfigurationDescriptor.HID_PanelHID;
				Size    = sizeof(USB_HID_Descriptor_HID_t);
				break;
			}
			#endif
			break;

		case DTYPE_Report:
			if (wIndex == IFACENUMBER_MISC)
			{
				Address = &MiscReport;
				Size    = sizeof(MiscReport);
				break;
			}
			#if defined(ENABLE_LED_DEVICE)
			if (wIndex == IFACENUMBER_LED)
			{
				Address = &LEDReport;
				Size    = sizeof(LEDReport);
				break;
			}
			#endif
			#if defined(ENABLE_PANEL_DEVICE)
			if (wIndex == IFACENUMBER_PANEL)
			{
				Address = &PanelReport;
				Size    = sizeof(PanelReport);
				break;
			}
			#endif
			break;
	}

	*DescriptorAddress = Address;

	return Size;
}
