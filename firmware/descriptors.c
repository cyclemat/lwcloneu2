/*
 * LWCloneU2
 * Copyright (C) 2013 Andreas Dittrich <lwcloneu2@cithraidt.de>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program;
 * if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#define USB_STRING_TABLE(_map_) \
	_map_(ManufacturerString_id,  "n/a") \
	_map_(ProductString_id,       "LWCloneU2 v1.0")


typedef enum {
	LanguageString_id = 0,
	#define MAP(x, y) x,
	USB_STRING_TABLE(MAP)
	#undef MAP
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


/** HID class report descriptor. This is a special descriptor constructed with values from the
 *  USBIF HID class specification to describe the reports and capabilities of the HID device. This
 *  descriptor is parsed by the host and its contents used to determine what data (and in what encoding)
 *  the device will send, and what it may be sent back from the host. Refer to the HID specification for
 *  more details on HID report descriptors.
 */

#if defined(ENABLE_PANEL_DEVICE)

enum HID_KeyModifierCodes
{
	KEY_LeftControl      = 0xE0,
	KEY_LeftShift,
	KEY_LeftAlt,
	KEY_LeftGUI,
	KEY_RightControl,
	KEY_RightShift,
	KEY_RightAlt,
	KEY_RightGUI
};

enum HID_ConsumerCodes
{
	CKEY_Mute       = 0xE2,
	CKEY_VolumeUp   = 0xE9,
	CKEY_VolumeDown = 0xEA,
};


const USB_Descriptor_HIDReport_Datatype_t PROGMEM PanelReport[] =
{
	#if (USE_KEYBOARD != 0)
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
	#endif

	#if (USE_CONSUMER != 0)
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
	#endif

	#if (NUM_JOYSTICKS >= 1)
	0x05, 0x01,             // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,             // USAGE (Gamepad)
	0xa1, 0x01,             // COLLECTION (Application)
	0x09, 0x01,             //   USAGE (Pointer)
	0xa1, 0x00,             //   COLLECTION (Physical)
	0x85, ID_Joystick1,     //     REPORT_ID (ID_Joystick1)
	0x09, 0x30,             //     USAGE (X)
	0x09, 0x31,             //     USAGE (Y)
	0x15, 0xFF,             //     LOGICAL_MINIMUM (-1)
	0x25, 0x01,             //     LOGICAL_MAXIMUM (1)
	0x75, 0x04,             //     REPORT_SIZE (4)
	0x95, 0x02,             //     REPORT_COUNT (2)
	0x81, 0x02,             //     INPUT (Data,Var,Abs)
	0x15, 0x00,             //     LOGICAL_MINIMUM (0)
	0x75, 0x01,             //     REPORT_SIZE (1)
	0x95, 0x08,             //     REPORT_COUNT (8)
	0x05, 0x09,             //     USAGE_PAGE (Button)
	0x19, 0x01,             //     USAGE_MINIMUM (Button 1)
	0x29, 0x08,             //     USAGE_MAXIMUM (Button 8)
	0x81, 0x02,             //     INPUT (Data,Var,Abs)
	0xc0,                   //   END_COLLECTION
	0xc0,                   // END_COLLECTION
	#endif

	#if (NUM_JOYSTICKS >= 2)
	0x05, 0x01,             // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,             // USAGE (Gamepad)
	0xa1, 0x01,             // COLLECTION (Application)
	0x09, 0x01,             //   USAGE (Pointer)
	0xa1, 0x00,             //   COLLECTION (Physical)
	0x85, ID_Joystick2,     //     REPORT_ID (ID_Joystick2)
	0x09, 0x30,             //     USAGE (X)
	0x09, 0x31,             //     USAGE (Y)
	0x15, 0xFF,             //     LOGICAL_MINIMUM (-1)
	0x25, 0x01,             //     LOGICAL_MAXIMUM (1)
	0x75, 0x04,             //     REPORT_SIZE (4)
	0x95, 0x02,             //     REPORT_COUNT (2)
	0x81, 0x02,             //     INPUT (Data,Var,Abs)
	0x15, 0x00,             //     LOGICAL_MINIMUM (0)
	0x75, 0x01,             //     REPORT_SIZE (1)
	0x95, 0x08,             //     REPORT_COUNT (8)
	0x05, 0x09,             //     USAGE_PAGE (Button)
	0x19, 0x01,             //     USAGE_MINIMUM (Button 1)
	0x29, 0x08,             //     USAGE_MAXIMUM (Button 8)
	0x81, 0x02,             //     INPUT (Data,Var,Abs)
	0xc0,                   //   END_COLLECTION
	0xc0,                   // END_COLLECTION
	#endif

	#if (NUM_JOYSTICKS >= 3)
	0x05, 0x01,             // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,             // USAGE (Gamepad)
	0xa1, 0x01,             // COLLECTION (Application)
	0x09, 0x01,             //   USAGE (Pointer)
	0xa1, 0x00,             //   COLLECTION (Physical)
	0x85, ID_Joystick3,     //     REPORT_ID (ID_Joystick3)
	0x09, 0x30,             //     USAGE (X)
	0x09, 0x31,             //     USAGE (Y)
	0x15, 0xFF,             //     LOGICAL_MINIMUM (-1)
	0x25, 0x01,             //     LOGICAL_MAXIMUM (1)
	0x75, 0x04,             //     REPORT_SIZE (4)
	0x95, 0x02,             //     REPORT_COUNT (2)
	0x81, 0x02,             //     INPUT (Data,Var,Abs)
	0x15, 0x00,             //     LOGICAL_MINIMUM (0)
	0x75, 0x01,             //     REPORT_SIZE (1)
	0x95, 0x08,             //     REPORT_COUNT (8)
	0x05, 0x09,             //     USAGE_PAGE (Button)
	0x19, 0x01,             //     USAGE_MINIMUM (Button 1)
	0x29, 0x08,             //     USAGE_MAXIMUM (Button 8)
	0x81, 0x02,             //     INPUT (Data,Var,Abs)
	0xc0,                   //   END_COLLECTION
	0xc0,                   // END_COLLECTION
	#endif

	#if (NUM_JOYSTICKS >= 4)
	0x05, 0x01,             // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,             // USAGE (Gamepad)
	0xa1, 0x01,             // COLLECTION (Application)
	0x09, 0x01,             //   USAGE (Pointer)
	0xa1, 0x00,             //   COLLECTION (Physical)
	0x85, ID_Joystick4,     //     REPORT_ID (ID_Joystick4)
	0x09, 0x30,             //     USAGE (X)
	0x09, 0x31,             //     USAGE (Y)
	0x15, 0xFF,             //     LOGICAL_MINIMUM (-1)
	0x25, 0x01,             //     LOGICAL_MAXIMUM (1)
	0x75, 0x04,             //     REPORT_SIZE (4)
	0x95, 0x02,             //     REPORT_COUNT (2)
	0x81, 0x02,             //     INPUT (Data,Var,Abs)
	0x15, 0x00,             //     LOGICAL_MINIMUM (0)
	0x75, 0x01,             //     REPORT_SIZE (1)
	0x95, 0x08,             //     REPORT_COUNT (8)
	0x05, 0x09,             //     USAGE_PAGE (Button)
	0x19, 0x01,             //     USAGE_MINIMUM (Button 1)
	0x29, 0x08,             //     USAGE_MAXIMUM (Button 8)
	0x81, 0x02,             //     INPUT (Data,Var,Abs)
	0xc0,                   //   END_COLLECTION
	0xc0,                   // END_COLLECTION
	#endif // NUM_JOYSTICKS >= 4

	#if (USE_MOUSE != 0)
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
	#endif
};

#endif


#if defined(ENABLE_LED_DEVICE)

#define LED_REPORT_SIZE 8

static const USB_Descriptor_HIDReport_Datatype_t PROGMEM LEDReport[] =
{
	HID_RI_USAGE_PAGE(16, 0xFF00), /* Vendor Page 0 */
	HID_RI_USAGE(8, 0x01), /* Vendor Usage 1 */
	HID_RI_COLLECTION(8, 0x01), /* Vendor Usage 1 */
	    HID_RI_USAGE(8, 0x02), /* Vendor Usage 2 */
	    HID_RI_LOGICAL_MINIMUM(8, 0x00),
	    HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
	    HID_RI_REPORT_SIZE(8, 0x08),
	    HID_RI_REPORT_COUNT(8, LED_REPORT_SIZE),
	    HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
	    HID_RI_USAGE(8, 0x03), /* Vendor Usage 3 */
	    HID_RI_LOGICAL_MINIMUM(8, 0x00),
	    HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
	    HID_RI_REPORT_SIZE(8, 0x08),
	    HID_RI_REPORT_COUNT(8, LED_REPORT_SIZE),
	    HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),
	HID_RI_END_COLLECTION(0),
};

#endif

/** Device descriptor structure. This descriptor describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
 
static USB_Descriptor_Device_t DeviceDescriptor =
{
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},
	.USBSpecification       = VERSION_BCD(01.10),
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,
	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
	.VendorID               = USB_VENDOR_ID,
	.ProductID              = USB_PRODUCT_ID,
	.ReleaseNumber          = USB_VERSION_BCD,
	.ManufacturerStrIndex   = ManufacturerString_id,
	.ProductStrIndex        = ProductString_id,
	.SerialNumStrIndex      = NO_DESCRIPTOR,
	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

void SetProductID(uint16_t id)
{
	DeviceDescriptor.ProductID = id;
}

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
 
#if defined(ENABLE_LED_DEVICE) && defined(ENABLE_PANEL_DEVICE)
	#define NUM_TOTAL_INTERFACES 2
	#define IFACENUMBER_PANEL 0
	#define IFACENUMBER_LED 1
#elif defined(ENABLE_LED_DEVICE) 
	#define NUM_TOTAL_INTERFACES 1
	#define IFACENUMBER_LED 0
#elif defined(ENABLE_PANEL_DEVICE)
	#define NUM_TOTAL_INTERFACES 1
	#define IFACENUMBER_PANEL 0
#else
	#error "invalid configuration, no led-device enabled and no panel-device enabled"
#endif
 
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
		.HIDSpec                = VERSION_BCD(01.11),
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
		.PollingIntervalMS      = 1
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
		.HIDSpec                = VERSION_BCD(01.11),
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
		.PollingIntervalMS      = 10
	},
	#endif
};

/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
 
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
			#if defined(ENABLE_LED_DEVICE)
			if (wIndex == IFACENUMBER_LED)
			{
				Address = &ConfigurationDescriptor.HID_LEDHID;
				Size    = sizeof(USB_HID_Descriptor_HID_t);
			}
			#endif
			#if defined(ENABLE_PANEL_DEVICE)
			if (wIndex == IFACENUMBER_PANEL) 
			{
				Address = &ConfigurationDescriptor.HID_PanelHID;
				Size    = sizeof(USB_HID_Descriptor_HID_t);
			}
			#endif
			break;

		case DTYPE_Report:
			#if defined(ENABLE_LED_DEVICE)
			if (wIndex == IFACENUMBER_LED)
			{
				Address = &LEDReport;
				Size    = sizeof(LEDReport);
			}
			#endif
			#if defined(ENABLE_PANEL_DEVICE)
			if (wIndex == IFACENUMBER_PANEL) 
			{
				Address = &PanelReport;
				Size    = sizeof(PanelReport);
			}
			#endif
		break;
	}

	*DescriptorAddress = Address;
	
	return Size;
}

