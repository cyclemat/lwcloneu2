/*
 * LWCloneU2 (Due) – Hardware-Hilfen: Uhr, Soft-PWM-Timer, ADC, EEPROM-Ersatz im Flash, Neustart
 * LWCloneU2 (Due) – hardware helpers: clock, soft PWM timer, ADC, EEPROM stand-in in flash, reboot
 */

#ifndef HW_DUE_H__INCLUDED
#define HW_DUE_H__INCLUDED

#include <stdint.h>
#include <stdbool.h>

/* EEPROM-Ersatz: 1 KB am Ende von Flash-Bank 1; eeprom_*() aus compat/avr/eeprom.h arbeiten darauf.
 * EEPROM stand-in: 1 KB at the end of flash bank 1; eeprom_*() from compat/avr/eeprom.h work on it. */
#define EE_SIZE          1024
#define EE_FLASH_ADDR    (IFLASH1_ADDR + IFLASH1_SIZE - EE_SIZE)

/* Neustart; enter_bootloader = true startet den SAM-BA-Bootloader im ROM (nativer Port meldet sich als
 * "Bossa Program Port"), wie der 1200-Baud-Trick der Arduino-IDE.
 * Reboot; enter_bootloader = true starts the ROM SAM-BA bootloader (native port shows up as
 * "Bossa Program Port"), like the Arduino IDE's 1200 baud touch. */
void hw_restart(bool enter_bootloader);

#endif
