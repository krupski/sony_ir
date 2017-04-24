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

#include "sony_ir.h"

// IR carrier generator & timer
ISR (TIMER0_COMPA_vect)
{
	if (count--) {
		// toggle IR LED on and off
		// Atmel datasheet 2586Q–AVR–08/2013 pg. 53
		enable ? IO_INP |= IR_BIT : 0;

	} else {
		busy = 0; // flag ISR is done
	}
}

// INT0 handler - we arrive here when the button is pressed
ISR (INT0_vect)
{
	GIMSK &= ~_BV(INT0); // disable hardware INT0
	sleep_disable(); // prevent going back to sleep
}

// msec delay using the IR timer interrupt (soft delays are
// inaccurate when an ISR is running and stealing CPU cycles!)
// valid delays are 0 to 65535 (0.0 to 65.535 seconds)
void delayMS (uint16_t msec)
{
	while (msec--) { // for each millisecond
		cli(); // disable interrupts
		busy = 1; // flag ISR busy
		enable = 0; // LED off
		count = (IR_FREQ / 500);
		sei(); // enable interrupts
		while (busy); // wait until timeout
	}
}

// send IR command from array
// *cmd -> pointer to timing data array
// repeat -> how many times to send the array
// delay -> number of milliseconds to delay between repeats
void sendCmd (const uint16_t *cmd, uint16_t repeat, uint16_t delay)
{
	uint16_t *ptr; // data block pointer
	uint16_t data; // read IR pulse width info

	IO_DDR |= IR_BIT; // LED pin as output

	while (repeat--) { // command repeat count, if any

		ptr = ((uint16_t *)(cmd)); // (re)init data pointer

		while (1) {
			data = _READ ((const uint16_t *)(ptr)); // read data

			if (data == 0) { // 0 flags end of data
				break;

			} else {
				ptr++;
			}

			cli(); // disable interrupts

			busy = 1; // flag ISR busy
			enable = (data >> 15); // bit 15 is LED on/off
			count = (data << 1); // count * 2 for full cycle and mask off enable flag

			sei(); // enable interrupts
			while (busy); // wait while ISR is sending
		}

		delayMS (delay); // inter-command delay, if any
	}

	IO_DDR &= ~IR_BIT; // LED pin as input
}

// setup timer 0
void timerInit (uint32_t rate)
{
	cli(); // disable interrupts

	enable = 0x00; // zero all...
	busy = 0x00; // ...ISR...
	count = 0x00; // ...vars

	// set timer 0 (8 bit) to CTC mode, carrier freq. this times the IR pulses.
	TCCR0A = _BV(WGM01); // mode 2 (CTC) OCR0A = top
	TCCR0B = _BV(CS00); // set F_CPU/1
	OCR0A  = ((F_CPU / (rate * 2)) - 1); // double rate because we toggle
	TIMSK  = _BV(OCIE0A); // enable interrupt on OCR0A match

	sei(); // interrupts on
}

int main (void)
{
	uint8_t n; // generic

	timerInit (IR_FREQ); // setup 8 bit timer for IR carrier frequency

	while (1) {

		cli(); // interrupts off

		GIMSK |= _BV(INT0); // enable INT0 to trigger active low

		set_sleep_mode (SLEEP_MODE_PWR_DOWN); // set full power down mode
		sleep_enable(); // enable sleep mode

		sei(); // interrupts on
		sleep_cpu(); // shut down cpu (awaiting button INT0)

		// debounce start button
		n = 10; // init debounce
		while (n--) {
			if (IO_INP & BUTTON) { // if button bounced up...
				n = 10; // ...reload debounce count
			}
			delayMS (1);
		}

		// send shutter command
		// data, repeat 3 times, no inter-command delay
		sendCmd (shutter, 3, 0);

		// wait for button to be RELEASED
		// (prevent repeat if button held down)
		n = 10; // init debounce
		while (n--) {
			if (! (IO_INP & BUTTON)) { // if button pressed...
				n = 10; // ...load debounce count
			}
			delayMS (1);
		}
	}
}
// end of sony_ir.cpp
