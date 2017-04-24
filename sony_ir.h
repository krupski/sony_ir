///////////////////////////////////////////////////////////////////////////////
//
//  Infrared Remote Control for SONY DSLR A-700 Digital Camera
//  Copyright (c) 2016, 2017 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 24 April 2017
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SONY_IR_H
#define SONY_IR_H

/////////////////////////////////////////////////////
// We are using an ATTiny85 processor @ 16.000 MHz.
/////////////////////////////////////////////////////

#include <avr/interrupt.h> // interrupt vectors & misc
#include <avr/sleep.h>     // CPU low power sleep mode support
#include <avr/power.h>     // CPU power save functions
#include <avr/eeprom.h>    // read data from EEMEM (i.e. eeprom)
#include <avr/pgmspace.h>  // read data from PROGMEM (i.e. flash)

#define IO_INP  PINB       // IO port read  _SFR_MEM8(0x16+0x20)
#define IO_DDR  DDRB       // IO port ddr   _SFR_MEM8(0x17+0x20)
#define IO_OUT PORTB       // IO port write _SFR_MEM8(0x18+0x20)

#define IR_BIT _BV(PB0)    // IR LED drive pin (pin 5)
#define SELECT _BV(PB1)    // code select (currently unused) (pin 6)
#define BUTTON _BV(PB2)    // send IR code (button pulls low) (pin 7)
#define IR_FREQ 40000      // IR carrier freq in hertz

volatile uint8_t enable;   // LED pulses on or off
volatile uint8_t   busy;   // ISR busy flag
volatile uint16_t count;   // led pulse counter

// function prototypes
void delayMS (uint16_t);
void sendCmd (const uint16_t *, uint16_t, uint16_t);
void timerInit (uint32_t);

///////////////////////////////////////////////////////////////////////////////
// Sony camera shutter release code 0xB4B8F (focus and shoot)
// Header + 20 bit (5 byte) command.
// bit 15 (0x8000) set = LED on
///////////////////////////////////////////////////////////////////////////////
#define _READ eeprom_read_word // sticking data in EEPROM
static const uint16_t shutter[] EEMEM = {
	// header
	0x8060, 0x0018,
	// data
	0x8030, 0x0018, 0x8018, 0x0018, 0x8030, 0x0018, 0x8030, 0x0018, // B
	0x8018, 0x0018, 0x8030, 0x0018, 0x8018, 0x0018, 0x8018, 0x0018, // 4
	0x8030, 0x0018, 0x8018, 0x0018, 0x8030, 0x0018, 0x8030, 0x0018, // B
	0x8030, 0x0018, 0x8018, 0x0018, 0x8018, 0x0018, 0x8018, 0x0018, // 8
	0x8030, 0x0018, 0x8030, 0x0018, 0x8030, 0x0018, 0x8030, 0x01C8, // F
	// end of data block
	0x0000,
};
/**
#define _READ pgm_read_word // sticking data in FLASH
static const uint16_t shutter[] PROGMEM = {
	// header
	0x8060, 0x0018,
	// data
	0x8030, 0x0018, 0x8018, 0x0018, 0x8030, 0x0018, 0x8030, 0x0018, // B
	0x8018, 0x0018, 0x8030, 0x0018, 0x8018, 0x0018, 0x8018, 0x0018, // 4
	0x8030, 0x0018, 0x8018, 0x0018, 0x8030, 0x0018, 0x8030, 0x0018, // B
	0x8030, 0x0018, 0x8018, 0x0018, 0x8018, 0x0018, 0x8018, 0x0018, // 8
	0x8030, 0x0018, 0x8030, 0x0018, 0x8030, 0x0018, 0x8030, 0x01C8, // F
	// end of data block
	0x0000,
};
**/
#endif // #ifndef SONY_IR_H

// end of sony_ir.h
