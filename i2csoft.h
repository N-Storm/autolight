/**********************************************************

Software I2C Library for AVR Devices.

Copyright 2008-2012
eXtreme Electronics, India
www.eXtremeElectronics.co.in
**********************************************************/


#ifndef _I2CSOFT_H
#define _I2CSOFT_H

#include "autolight.h"

/* 
I/O Configuration 
*/

#define SCLPORT	PORTB	//TAKE PORTD as SCL OUTPUT WRITE
#define SCLDDR	DDRB	//TAKE DDRB as SCL INPUT/OUTPUT configure

#define SDAPORT	PORTB	//TAKE PORTD as SDA OUTPUT WRITE
#define SDADDR	DDRB	//TAKE PORTD as SDA INPUT configure

#define SDAPIN	PINB	//TAKE PORTD TO READ DATA
#define SCLPIN	PINB	//TAKE PORTD TO READ DATA

#define SDAPUE	PUEB
#define SCLPUE	PUEB

#define SCL	PINB0	//PORTD.0 PIN AS SCL PIN
#define SDA	PINB1	//PORTD.1 PIN AS SDA PIN


// We have to use external pullups with RECHECK_AL due to code size limits
#ifdef RECHECK_AL
	#define SOFT_I2C_SDA_LOW	SDADDR |=  (1 << SDA)
	#define SOFT_I2C_SDA_HIGH	SDADDR &= ~(1 << SDA)
	#define SOFT_I2C_SCL_LOW	SCLDDR |=  (1<<SCL)
	#define SOFT_I2C_SCL_HIGH	SCLDDR &= ~(1<<SCL)
#else
	#define SOFT_I2C_SDA_LOW	do { SDAPUE &= ~(1 << SDA); SDADDR |= (1 << SDA); } while (0) // Turn off pull-up & set port to OUTPUT (sink)
	#define SOFT_I2C_SDA_HIGH	do { SDADDR &= ~(1 << SDA); SDAPUE |= (1 << SDA); } while (0) // Set port to INPUT & turn on pull-up
	#define SOFT_I2C_SCL_LOW	do { SCLPUE &= (~1 << SCL); SCLDDR |= (1 << SCL); } while (0)
	#define SOFT_I2C_SCL_HIGH	do { SCLDDR &= ~(1 << SCL); SCLPUE |= (1 << SCL); } while (0)
#endif

/**********************************************************
SoftI2CInit()

Description:
	Initializes the Soft I2C Engine.
	Must be called before using any other lib functions.
	
Arguments:
	NONE
	
Returns:
	Nothing

**********************************************************/
void SoftI2CInit();	

/**********************************************************
SoftI2CStart()

Description:
	Generates a START(S) condition on the bus.
	NOTE: Can also be used for generating repeat start(Sr)
	condition too.
	
Arguments:
	NONE
	
Returns:
	Nothing

**********************************************************/
void SoftI2CStart();

/**********************************************************
SoftI2CStop()

Description:
	Generates a STOP(P) condition on the bus.
	NOTE: Can also be used for generating repeat start
	condition too.
	
Arguments:
	NONE
	
Returns:
	Nothing

**********************************************************/
void SoftI2CStop();

/**********************************************************
SoftI2CWriteByte()

Description:
	Sends a Byte to the slave.
	
Arguments:
	8 bit date to send to the slave.
	
Returns:
	non zero if slave acknowledge the data receipt.
	zero other wise.

**********************************************************/
uint8_t SoftI2CWriteByte(uint8_t data);

/**********************************************************
SoftI2CReadByte()

Description:
	Reads a byte from slave.
	
Arguments:
	1 if you want to acknowledge the receipt to slave.
	0 if you don't want to acknowledge the receipt to slave.
	
Returns:
	The 8 bit data read from the slave.

**********************************************************/
uint8_t SoftI2CReadByte(uint8_t ack);


#endif
