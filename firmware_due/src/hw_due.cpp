/*
 * LWCloneU2 (Due) – Hardware-Hilfen: Uhr, Soft-PWM-Timer, ADC, EEPROM-Ersatz im Flash, Neustart
 * LWCloneU2 (Due) – hardware helpers: clock, soft PWM timer, ADC, EEPROM stand-in in flash, reboot
 *
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

#include <Arduino.h>
#include <string.h>
#include "lwc_shared.h"
#include "hw_due.h"
#include <avr/eeprom.h>


/* ---------------------------------------------------------------- Uhr / clock */

extern "C" uint16_t clock_ms(void)
{
	return (uint16_t)millis();
}


/* ---------------------------------------------------------------- Soft-PWM-Timer: TC1 Kanal 0 (TC3), 200 µs */

extern "C" void led_timer_init(void)
{
	pmc_enable_periph_clk(ID_TC3);

	/* TIMER_CLOCK4 = MCK/128 = 656,25 kHz → RC = 131 ≈ 199,6 µs (AVR: 200 µs) */
	TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4);
	TC_SetRC(TC1, 0, (VARIANT_MCK / 128) / 5000);

	TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
	TC1->TC_CHANNEL[0].TC_IDR = ~TC_IER_CPCS;

	NVIC_ClearPendingIRQ(TC3_IRQn);
	NVIC_SetPriority(TC3_IRQn, 1);       /* vor USB (Standard 0 ist höchste – USB bleibt wichtiger) / below USB */
	NVIC_EnableIRQ(TC3_IRQn);

	TC_Start(TC1, 0);
}

extern "C" void TC3_Handler(void)
{
	TC_GetStatus(TC1, 0);     /* Statusregister lesen = quittieren / reading acknowledges */
	outputs_tick();
}


/* ---------------------------------------------------------------- ADC: Freilauf auf den gewählten Kanälen */

extern "C" void ADC_init(void)
{
	pmc_enable_periph_clk(ID_ADC);
	adc_init(ADC, SystemCoreClock, ADC_FREQ_MAX, ADC_STARTUP_FAST);
	adc_configure_timing(ADC, 15, ADC_SETTLING_TIME_3, 1);
	adc_set_resolution(ADC, ADC_12_BITS);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 1);       /* 1 = Freilauf / free run */
	adc_disable_interrupt(ADC, 0xFFFFFFFF);
	ADC->ADC_CHDR = 0xFFFF;
}

extern "C" void ADC_select(const uint8_t *channels, uint8_t n)
{
	ADC->ADC_CHDR = 0xFFFF;

	for (uint8_t i = 0; i < n; i++)
		if (channels[i] < 16)
			ADC->ADC_CHER = (1u << channels[i]);

	if (n > 0)
		adc_start(ADC);
}

extern "C" uint16_t ADC_read(uint8_t channel)
{
	if (channel >= 16)
		return 0;

	return (uint16_t)(ADC->ADC_CDR[channel] >> 2);     /* 12 Bit → 10 Bit wie AVR / like AVR */
}


/* ---------------------------------------------------------------- EEPROM-Ersatz / EEPROM stand-in */

/*
 * Der Block liegt am Ende von Bank 1 (EFC1), die Firmware läuft aus Bank 0 – Bank 1 kann deshalb
 * beschrieben werden, ohne dass der Code anhält. Geschrieben wird seitenweise (256 Byte) mit
 * "Erase page and write page" (EWP); vorher wird die Sperrregion freigegeben.
 *
 * The block sits at the end of bank 1 (EFC1), the firmware runs from bank 0 – so bank 1 can be
 * written without stalling code execution. Writing is page-wise (256 bytes) with "erase page and
 * write page" (EWP); the lock region is cleared first.
 */

static uint8_t s_ee[EE_SIZE];
static bool    s_ee_loaded = false;

static void ee_load(void)
{
	if (!s_ee_loaded)
	{
		memcpy(s_ee, (const void *)EE_FLASH_ADDR, EE_SIZE);
		s_ee_loaded = true;
	}
}

/* EFC-Befehl an Bank 1; läuft aus dem RAM, damit währenddessen kein Flash-Zugriff nötig ist
 * EFC command to bank 1; runs from RAM so no flash access is needed meanwhile */
__attribute__((section(".ramfunc"), noinline, long_call))
static uint32_t efc1_command(uint32_t cmd, uint32_t arg)
{
	while ((EFC1->EEFC_FSR & EEFC_FSR_FRDY) == 0) {}
	EFC1->EEFC_FCR = EEFC_FCR_FKEY(0x5A) | EEFC_FCR_FARG(arg) | EEFC_FCR_FCMD(cmd);
	uint32_t status;
	do {
		status = EFC1->EEFC_FSR;
	} while ((status & EEFC_FSR_FRDY) == 0);
	return status;
}

static bool ee_store(void)
{
	uint32_t const first_page = (EE_FLASH_ADDR - IFLASH1_ADDR) / IFLASH1_PAGE_SIZE;
	uint32_t const pages = EE_SIZE / IFLASH1_PAGE_SIZE;
	bool ok = true;

	uint32_t const primask = __get_PRIMASK();
	__disable_irq();

	efc1_command(EFC_FCMD_CLB, first_page);         /* Sperrregion (16 KB) freigeben / unlock region */

	for (uint32_t p = 0; p < pages; p++)
	{
		volatile uint32_t * const latch = (volatile uint32_t *)(EE_FLASH_ADDR + p * IFLASH1_PAGE_SIZE);
		const uint32_t * const src = (const uint32_t *)(s_ee + p * IFLASH1_PAGE_SIZE);

		/* Seite liegt schon so im Flash? Dann sparen wir den Schreibzyklus / page unchanged? skip */
		if (memcmp((const void *)latch, src, IFLASH1_PAGE_SIZE) == 0)
			continue;

		for (uint32_t i = 0; i < IFLASH1_PAGE_SIZE / 4; i++)
			latch[i] = src[i];                       /* Latch-Puffer füllen / fill the latch buffer */

		uint32_t const status = efc1_command(EFC_FCMD_EWP, first_page + p);

		if (status & (EEFC_FSR_FCMDE | EEFC_FSR_FLOCKE))
			ok = false;
	}

	__set_PRIMASK(primask);
	return ok;
}

static bool ee_range_ok(const void *addr, size_t n)
{
	uintptr_t const off = (uintptr_t)addr;
	return off < EE_SIZE && n <= EE_SIZE - off;
}

extern "C" void eeprom_read_block(void *dst, const void *src_addr, size_t n)
{
	ee_load();

	if (ee_range_ok(src_addr, n))
		memcpy(dst, &s_ee[(uintptr_t)src_addr], n);
	else
		memset(dst, 0xFF, n);
}

extern "C" void eeprom_update_block(const void *src, void *dst_addr, size_t n)
{
	ee_load();

	if (!ee_range_ok(dst_addr, n))
		return;

	uint8_t * const dst = &s_ee[(uintptr_t)dst_addr];

	if (memcmp(dst, src, n) == 0)
		return;

	memcpy(dst, src, n);
	ee_store();
}

extern "C" void eeprom_write_block(const void *src, void *dst_addr, size_t n)
{
	eeprom_update_block(src, dst_addr, n);
}

extern "C" uint8_t eeprom_read_byte(const uint8_t *addr)
{
	uint8_t v;
	eeprom_read_block(&v, addr, 1);
	return v;
}

extern "C" void eeprom_update_byte(uint8_t *addr, uint8_t value)
{
	eeprom_update_block(&value, addr, 1);
}

extern "C" void eeprom_write_byte(uint8_t *addr, uint8_t value)
{
	eeprom_update_block(&value, addr, 1);
}


/* ---------------------------------------------------------------- Neustart / reboot */

extern "C" void banzai(void);     /* Reset.cpp des Cores: GPNVM1 löschen + Reset → SAM-BA / core: clear GPNVM1 + reset */

void hw_restart(bool enter_bootloader)
{
	USBDevice.detach();
	delay(250);                   /* der Host soll das Abmelden sehen / let the host notice the detach */

	if (enter_bootloader)
		banzai();

	__disable_irq();
	RSTC->RSTC_CR = RSTC_CR_KEY(0xA5) | RSTC_CR_PROCRST | RSTC_CR_PERRST;

	for (;;) {}
}
