/*
 * LWCloneU2 (Due) – die gemeinsamen C-Module aus ../firmware mit C-Bindung einbinden
 * LWCloneU2 (Due) – include the shared C modules from ../firmware with C linkage
 */

#ifndef LWC_SHARED_H__INCLUDED
#define LWC_SHARED_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <hwconfig.h>
#include "pins.h"
#include "config.h"
#include "proto.h"
#include "outputs.h"
#include "panel.h"
#include "strip.h"
#include "strip_serial.h"
#include "ws2812.h"

uint16_t clock_ms(void);                       /* clock.h (AVR-Teil nicht benutzt) / clock.h (AVR part unused) */

#ifdef __cplusplus
}
#endif

#endif
