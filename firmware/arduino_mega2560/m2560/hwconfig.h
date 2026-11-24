/*
 * LWCloneU2 – Arduino Mega 2560, Haupt-MCU ATmega2560 (hinter dem ATmega16U2)
 *
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

#ifndef HWCONFIG_H__INCLUDED
#define HWCONFIG_H__INCLUDED

#include <stdint.h>
#include <avr/io.h>

#if (F_CPU != 16000000)
#error "invalid CPU clock frequency ==> should be 16 MHZ"
#endif

#if !defined(__AVR_ATmega2560__ )
#error "invalid CPU type ==> should be ATMega2560"
#endif

#include "../devconfig.h"
#include "pinmap.h"


/****************************************
 LED driver config (Timer 0, 200 µs)
****************************************/

#if defined(ENABLE_LED_DEVICE)

#define LED_TIMER_vect TIMER0_COMPA_vect

static void inline led_timer_init(void)
{
	const int T0_CYCLE_US = 200;
	OCR0A = (((T0_CYCLE_US * (F_CPU / 1000L)) / (64 * 1000L)) - 1);
	TCCR0A = _BV(WGM01); // clear timer/counter on compare0 match
	TCCR0B = _BV(CS01) |_BV(CS00); // prescale 64
	TIMSK0 = _BV(OCIE0A); // enable Output Compare 0 overflow interrupt
	TCNT0 = 0x00;
}

#endif


/****************************************
 Panel config
****************************************/

#if defined(ENABLE_PANEL_DEVICE)
#define PANEL_TASK
#endif


/****************************************
 ADC config
****************************************/

static void inline ADC_init(void)
{
	ADMUX = (1 << REFS0); // AVCC als Referenz / AVCC as reference

	ADCSRA =
		(1 << ADEN) |
		(1 << ADIE) |
		(1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128
}

// Datenblatt ATmega2560, 25.2.1: ADC8..15 über MUX5 in ADCSRB (Bit 3)
#define ADCSRB_MUX5 3

static void inline ADC_setmux(uint8_t mux)
{
	ADMUX = (uint8_t)((ADMUX & ~0x1F) | (mux & 0x1F));
	ADCSRB = (uint8_t)((ADCSRB & ~(1 << ADCSRB_MUX5)) | (((mux >> 5) & 0x01) << ADCSRB_MUX5));
}


/****************************************
 Clock config (Timer 1, 1 ms)
****************************************/

#define CLOCK_COMPARE_MATCH_vect TIMER1_COMPA_vect
#define CLOCK_TCNT TCNT1
#define CLOCK_OCR OCR1A

static void inline clock_init(void)
{
	OCR1A = TCNT1 + (F_CPU / 1000);
	TCCR1B = _BV(CS10); //  normal mode, no prescale
	TIMSK1 = _BV(OCIE1A); // enable Output Compare 1 interrupt
}


/****************************************
 Data UART config (UART0 → ATmega16U2)
****************************************/

#define DATA_TX_UART_vect   USART0_UDRE_vect
#define DATA_RX_UART_vect   USART0_RX_vect

#include "../../data_uart0.h"

static void inline data_uart_init(void)
{
	UBRR0 = 3; // 250 kBit/s @ 16 MHz CPU
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00) | (1 << UPM01) | (1 << UPM00); // asynchron uart, odd parity, 9-bit-character, 1 stop bit
	UCSR0B |= (1 << UCSZ02) | (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0); // enable Receiver and Transmitter
}


/****************************************
 Debug UART config (UART1 = D18/D19)
****************************************/

#if defined(DEBUGLEVEL)

#define DEBUG_TX_UART_vect   USART1_UDRE_vect

static void inline debug_uart_setUDRIE(uint8_t x) { if (x) { UCSR1B |= (1 << UDRIE1); } else { UCSR1B &= ~(1 << UDRIE1); } }
static void inline debug_uart_writeUDR(uint8_t x) { UDR1 = x; }

static void inline debug_uart_init(void)
{
	UBRR1 = 16;  // 115200 bit/s @ 16MHz (==> 2.1% error)
	UCSR1A |= (1 << U2X1);
	UCSR1C = (3 << UCSZ10); // 1 stop, 8 data
	UCSR1B |= (1 << TXEN1);
}

#endif

#endif
