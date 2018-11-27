/*
 * autolight.h
 *
 * Project: autolight
 * Author: NStorm
 * License: Apache License Version 2.0, January 2004 http://www.apache.org/licenses/
 * https://github.com/N-Storm/autolight
 */


#ifndef AUTOLIGHT_H_
#define AUTOLIGHT_H_

// Settings. PS = proximity sensor, AL = ambient light.
// Trigger interrupt/closed state when proximity value crosses PROX_TH
#define PROX_TH 30
// Set light on if CH0-CH1*2 less than that value (integer math approximation)
#define LIGHT_MAX 100
// Wait time 202 ms
#define WTIME_DEFAULT 0xB6
// PS Pulse count
#define PPULSE_DEFAULT 4
// PPERS = 3 consecutive PS values out of range
#define PERS_CON 0b00110000
// Delay after waking from sleep (on interrupt) before measuring AL level
#define ON_DELAY 600
// ALS Timing recommended ~50ms value, 19 cyc, 51.87ms
#define ATIME_DEFAULT 0xED
// PS Timing, recommended to be set to 0xFF (1 cyc, 10 bits)
#define PTIME_DEFAULT 0xFF

// Addresses and registers. 7-bit address value shifted left to MSB.
#define APDS_ADDR (0x39 << 1)

#define APDS_ENABLE   0x00
#define APDS_ATIME    0x01
#define APDS_PTIME    0x02
#define APDS_WTIME    0x03
#define APDS_PILTL    0x08
#define APDS_PILTH    0x09
#define APDS_PIHTL    0x0A
#define APDS_PIHTH    0x0B
#define APDS_PERS     0x0C
#define APDS_CONFIG   0x0D
#define APDS_PPULSE   0x0E
#define APDS_CONTROL  0x0F
#define APDS_ID       0x12
#define APDS_ALS      0x14
#define APDS_Ch0DATAL 0x14
#define APDS_Ch0DATAH 0x15
#define APDS_Ch1DATAL 0x16
#define APDS_Ch1DATAH 0x17
#define APDS_PROX     0x18
#define APDS_PDATAL   0x18
#define APDS_PDATAH   0x19

#define APDS_ID_VAL 0x39

// Enable register flags
#define SAI (1<<6) // Sleep after interrupt. When asserted, the device will power down at the end of a proximity or ALS cycle if an interrupt has been generated.
#define PIEN (1<<5) // Proximity Interrupt Mask. When asserted, permits proximity interrupts to be generated.
#define AIEN (1<<4) // ALS Interrupt Mask. When asserted, permits ALS interrupt to be generated.
#define WEN (1<<3) // Enable Wait
#define PEN (1<<2) // Enable Prox
#define AEN (1<<1) // Enable ALS
#define PON (1<<0) // Enable Power On

// Control register flags
// #define PDRIVE 0 // 100mA of LED Power
#define PDRIVE 0x80 // 25mA of LED Power
#define PDIODE 0x20 // CH1 Diode
#define PGAIN  0x00 // 1x Prox gain
#define AGAIN  0x01 // 8x ALS gain

// Command register flags
#define APDS_RESET_PROX_INT     0b11100101
#define REPEATED_BYTE           0x80
#define AUTO_INCREMENT          0xA0
#define SPECIAL_FN              0xE0

// States
#define RS_START      0
#define RS_SLEEP      1
#define RS_WAKE       2
#define RS_LOWLIGHT   3
#define RS_HIGHLIGHT  4
#define RS_CLOSED     5

// Utility macros
#define DNOP() _NOP(); _NOP();
#define QNOP() DNOP(); DNOP();

#endif /* AUTOLIGHT_H_ */
