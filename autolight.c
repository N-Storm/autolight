/*
 * auto-light.c
 *
 * Project: autolight
 * Author: NStorm
 * License: Apache License Version 2.0, January 2004 http://www.apache.org/licenses/
 * https://github.com/N-Storm/autolight
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

volatile uint8_t runstate = RS_START;
uint16_t al, prox;

ISR(INT0_vect) {
	sleep_disable();
	runstate = RS_WAKE;
}

void init() {
	RSTFLR = 0;
	wdt_disable();
	cli(); // Disable interrupts
	CCP = CCP_MAGIC; // Magic number to enable configuration access
	CLKMSR = 0b00; // Internal osc
	QNOP();
	CCP = CCP_MAGIC;
	CLKPSR = (1 << CLKPS2) | (1 << CLKPS0); // Prescaler 1:32 = 250 kHz system clock
	ACSR |= (1 << ACD); // ACD (ACD: Analog Comparator Disable = 1)
	PRR = (1 << PRADC) | (1 << PRTIM0); // Power Reduction Register
	sei(); // Enable interrupts

	PORTB = 0;

	// Turn on pull-up on interrupt pin if enabled in config.
	#ifdef INT_PULLUP
		PUEB = (1 << PUEB2);
	#endif

	DDRB |= (1 << DDB3); // PB3 - OUTPUT LOW

	EICRA |= (1 << ISC01) | (0 << ISC00); // The falling edge of INT0 generates an interrupt request.
	EIMSK |= (1 << INT0); // Enable INT0

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

// This function will send required "heading" before reading bytes from I2C
bool apds_readstart(uint8_t reg) {
	SoftI2CStart();
	// Send device addr for write
	if (!SoftI2CWriteByte(APDS_ADDR)) {
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
	// Send device addr + read flag
	if (!SoftI2CWriteByte(APDS_ADDR | 1)) {
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
	if (!SoftI2CWriteByte(APDS_ADDR)) {
		SoftI2CStop();
		//I2C i/o error
		return false;
	}
	SoftI2CWriteByte(reg | AUTO_INCREMENT);
	SoftI2CWriteByte(val);
	SoftI2CStop();

	return true;
}

/* Read AL data to al global var.
 * For some reason APDS-9930 auto increment reading of values won't work for me. 
 * So we read one byte at once only.
 */
bool apds_readal() {
	uint16_t ch0, ch1;
	
	if (!apds_readstart(APDS_Ch0DATAL))
		return false;
	ch0 = SoftI2CReadByte(0);

	if (!apds_readstart(APDS_Ch0DATAH))
		return false;
	ch0 += SoftI2CReadByte(0) << 8;

	if (!apds_readstart(APDS_Ch1DATAL))
		return false;
	ch1 = SoftI2CReadByte(0);

	if (!apds_readstart(APDS_Ch1DATAH))
		return false;
	ch1 += SoftI2CReadByte(0) << 8;

	SoftI2CStop();

	if (ch0 < (ch1*2)) {
		if (ch0 > LIGHT_TH*2)
			al = ch0; // IR overflow
		else
			al = 0;
	} else
		al = ch0 - (ch1*2);

	return true;
}

/* Read PS data to prox global var.
 * For some reason APDS-9930 auto increment reading of values won't work for me.
 * So we read one byte at once only.
 */
bool apds_readprox() {
	if (!apds_readstart(APDS_PDATAL))
		return false;
	prox = SoftI2CReadByte(0);

	if (!apds_readstart(APDS_PDATAH))
		return false;
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

	apds_writebyte(APDS_RESET_PROX_INT, 0); // Clear PS Interrupt just in case we've restarted
	apds_writebyte(APDS_ATIME, ATIME_DEFAULT);
	apds_writebyte(APDS_PTIME, PTIME_DEFAULT);
	apds_writebyte(APDS_WTIME, WTIME_DEFAULT);
	apds_writebyte(APDS_PPULSE, PPULSE_DEFAULT);
	apds_writebyte(APDS_CONTROL, PDRIVE | PDIODE | PGAIN | AGAIN);
	apds_writebyte(APDS_PILTL, (uint8_t)PROX_TH);
	apds_writebyte(APDS_PILTH, PROX_TH >> 8);
	apds_writebyte(APDS_PIHTL, 0); // Write zero to disable high threshold
	apds_writebyte(APDS_PIHTH, 0);
	apds_writebyte(APDS_CONFIG, 0); // Reset to default
	apds_writebyte(APDS_PERS, PERS_CON);
	apds_writebyte(APDS_ENABLE, WEN | PEN | PON);

	return true;
}

// Reset via WDT on error
void reset() {
	wdt_enable(WDTO_15MS);
	cli();
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
			apds_writebyte(APDS_RESET_PROX_INT, 0); // Clear PS Interrupt
			sei();
			if (!apds_writebyte(APDS_ENABLE, PIEN | WEN | PEN | PON))
				reset();

			sleep_enable();
			sleep_cpu();
			sleep_disable();
		}
		if (runstate == RS_WAKE) {
			if (!apds_writebyte(APDS_ENABLE, AEN | PON))
				reset();
			_delay_ms(DELAY);

			if (!apds_readal())
				reset();
			if (al < LIGHT_TH)
				runstate = RS_LOWLIGHT;
			else
				runstate = RS_HIGHLIGHT;
		}
		if (runstate == RS_LOWLIGHT || runstate == RS_HIGHLIGHT) {
			if (runstate == RS_LOWLIGHT)
				LIGHTON(); // Turn lights on
			#ifdef RECHECK_AL
				if (!apds_writebyte(APDS_ENABLE, WEN | PEN | AEN | PON))
					reset();
			#else // not defined RECHECK_AL
				if (!apds_writebyte(APDS_ENABLE, WEN | PEN | PON))
					reset();
			#endif // RECHECK_AL

			_delay_ms(DELAY);

			if (!apds_readprox())
				reset();
			if (prox > PROX_TH)
				runstate = RS_CLOSED;
			#ifdef RECHECK_AL
				else {
					if (!apds_readal())
						reset();
					if (al < LIGHT_TH)
						runstate = RS_LOWLIGHT;
					else
						runstate = RS_HIGHLIGHT;
				}
			#endif // RECHECK_AL
		}
		if (runstate == RS_CLOSED) {
			LIGHTOFF(); // Turn lights off
			cli();
			runstate = RS_SLEEP;
		}
	}
}
