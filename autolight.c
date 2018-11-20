/*
 * auto-light.c
 *
 * Project: autolight
 * Author: NStorm
 * License: Apache License Version 2.0, January 2004 http://www.apache.org/licenses/
 */

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "i2csoft.h"
#include "autolight.h"

uint8_t runstate = RS_START;
uint16_t al, prox;

ISR(INT0_vect) {
	runstate = RS_WAKE;
}

void init() {
	cli(); // Disable interrupts
	CCP = 0xD8; // Magic number to enable configuration access
	CLKMSR = 0b00; // Internal osc
	QNOP();
	CCP = 0xD8;
	CLKPSR = 0b0101; // Prescaler 1:32 = 250 kHz system clock
	// CLKPSR = 0; // Prescaler 1:1 = 8 MHz system clock
	wdt_disable();
	ACSR |= (1 << 7); // ACD (ACD: Analog Comparator Disable = 1)
	PRR = 0b11; // Power Reduction Register
	sei(); // Enable interrupts

	DDRB |= (1 << DDB3); // PB3 - OUTPUT

	// PCICR = (1 << PCIE0); // Enable port change interrupt
	// PCMSK = (1 << PCINT0) | (1 << PCINT1); // Enable interrupt on PB0, PB1 change
	EICRA |= (1 << ISC00) | (1 << ISC01); // The rising edge of INT0 generates an interrupt request.
	EIMSK |= (1 << INT0); // Enable INT0

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

bool apds_readstart(uint8_t reg) {
	SoftI2CStart();
	// Send device addr + write
	if (!SoftI2CWriteByte(APDS_ADDR+1)) {
		SoftI2CStop();
		//I2C i/o error
		return false;
	}
	// Send register addr + auto increment
	if (!SoftI2CWriteByte(reg | AUTO_INCREMENT)) {
		SoftI2CStop();
		return false;
	}
	SoftI2CStart(); // Repeated start
	// Send device addr + read
	if (!SoftI2CWriteByte(APDS_ADDR)) {
		SoftI2CStop();
		return false;
	}
	return true;
}

bool apds_readbyte(uint8_t reg, uint8_t *val) {
	if (!apds_readstart(reg))
		return false;

	*val = SoftI2CReadByte(0); // NACK = end transmission
	SoftI2CStop();

	return true;
}

bool apds_writebyte(uint8_t reg, uint8_t val) {
	SoftI2CStart();
	if (!SoftI2CWriteByte(APDS_ADDR+1)) {
		SoftI2CStop();
		//I2C i/o error
		return false;
	}
	SoftI2CWriteByte(reg | AUTO_INCREMENT);
	SoftI2CWriteByte(val);
	SoftI2CStop();

	return true;
}

bool apds_readal() {
	uint16_t ch0, ch1;
	
	if (!apds_readstart(APDS_ALS))
		return false;

	ch0 = SoftI2CReadByte(1);
	ch0 += SoftI2CReadByte(1) << 8;
	ch1 = SoftI2CReadByte(1);
	ch1 += SoftI2CReadByte(0) << 8;
	SoftI2CStop();
	if (!ch0 || !ch1)
		return false;
	else
		al = ch0 - (ch1*2);

	return true;
}

bool apds_readprox() {
	if (!apds_readstart(APDS_PROX))
	return false;

	prox = SoftI2CReadByte(1);
	prox += SoftI2CReadByte(0) << 8;
	SoftI2CStop();

	return true;
}

bool apds_init() {
	uint8_t id = 0;
	
	// Read & check sensor ID
	if (!apds_readbyte(APDS_ID, &id))
		return false;
	if (id != APDS_ID_VAL)
		return false;	
	
	apds_writebyte(APDS_ATIME, 0xFF);
	apds_writebyte(APDS_PTIME, 0xFF);
	apds_writebyte(APDS_WTIME,	WTIME_DEFAULT);
	apds_writebyte(APDS_PPULSE, PPULSE_DEFAULT);
	apds_writebyte(APDS_CONTROL, PDRIVE | PDIODE | PGAIN | AGAIN);
	apds_writebyte(APDS_PILTL, (uint8_t)PROX_TH);
	apds_writebyte(APDS_PILTH, PROX_TH >> 8);
	apds_writebyte(APDS_PERS, PERS_CON);
	apds_writebyte(APDS_ENABLE, PIEN | WEN | PEN | PON);
		
	return true;
}

// Reset via WDT on error
void reset() {
	wdt_enable(WDTO_15MS);
	while (1);
}

int main(void) {	
	init();
	SoftI2CInit();
	
	if (!apds_init())
		reset();	
	runstate = RS_SLEEP;

	while (1)
	{
		if (runstate == RS_SLEEP) {
			sleep_enable();
			sei();
			sleep_cpu();
			sleep_disable();
		}
		if (runstate == RS_WAKE) {
			if (!apds_writebyte(APDS_ENABLE, AEN | PON))
				reset();
			QNOP();
			_delay_ms(600);
			if (!apds_readal())
				reset();
			if (!apds_writebyte(APDS_ENABLE, WEN | PEN | PON))
				reset();
			if (al < LIGHT_MAX)
				runstate = RS_LOWLIGHT;
			else
				runstate = RS_HIGHLIGHT;
		}
		if (runstate == RS_LOWLIGHT) {
			PORTB |= (1 << PB3); // Turn lights on
			if (!apds_readprox())
				reset();
			if (prox > PROX_TH)
				runstate = RS_CLOSED;
		}
		if (runstate == RS_HIGHLIGHT) {
			if (!apds_readprox())
				reset();
			if (prox > PROX_TH)
				runstate = RS_CLOSED;
		}
		if (runstate == RS_CLOSED) {
			PORTB &= ~(1 << PB3);
			if (!apds_writebyte(APDS_ENABLE, PIEN | WEN | PEN | PON))
				reset();
			runstate = RS_SLEEP;
		}		
	}
}
