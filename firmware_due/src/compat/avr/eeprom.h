/*
 * LWCloneU2 (Due) – Ersatz für <avr/eeprom.h>: der SAM3X8E hat kein EEPROM, die Daten liegen in einem
 * 1-KB-Block am Ende von Flash-Bank 1 und werden in hw_due.cpp gespiegelt (RAM) und bei Änderung
 * seitenweise geschrieben. "Adressen" sind Offsets 0..E2END in diesem Block.
 *
 * LWCloneU2 (Due) – replacement for <avr/eeprom.h>: the SAM3X8E has no EEPROM, the data lives in a
 * 1 KB block at the end of flash bank 1, mirrored in RAM by hw_due.cpp and written page-wise on
 * change. "Addresses" are offsets 0..E2END into that block.
 */

#ifndef LWC_COMPAT_AVR_EEPROM_H
#define LWC_COMPAT_AVR_EEPROM_H

#include <stdint.h>
#include <stddef.h>

#define E2END   1023
#define EEMEM

#ifdef __cplusplus
extern "C" {
#endif

void    eeprom_read_block(void *dst, const void *src_addr, size_t n);
void    eeprom_write_block(const void *src, void *dst_addr, size_t n);
void    eeprom_update_block(const void *src, void *dst_addr, size_t n);
uint8_t eeprom_read_byte(const uint8_t *addr);
void    eeprom_write_byte(uint8_t *addr, uint8_t value);
void    eeprom_update_byte(uint8_t *addr, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
