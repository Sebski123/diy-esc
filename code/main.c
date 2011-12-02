/* 
 * qut-brushless-controller, an open-source Brushless DC motor controller
 * Copyright (C) 2011 Toby Lockley <tobylockley@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//===================================//
//      Main Controller File         //
//===================================//
//Initialisation and main program code
#include <avr/io.h>
#include <avr/interrupt.h>

//Pin definitions and config files
#include "config/BLDC_pindefs_breadboard.h"
#include "config/BLDC_motor_A2208.h"
#include "config/BLDC_config.h"

//Controller is broken up into easy to manage stubs
#include "rc_signal.h"

//extern volatile	uint16_t	signalBuffer; //Received signal is stored in here; When no signal, this = 0
void setPWMFromSignal(uint32_t sig);

int main(void)
{
	DDRB  = (1 << PB3);

	//clk/8, fastPWM, toggle OC2
	TCCR2 |= ( (1 << WGM20)
			 | (1 << COM21)
			 | (1 << WGM21)
			 | (1 << CS21));
			 
	OCR2 = 0;
	init_rc();
	sei(); //Enable global interrupts
	
	while(1) {
		if (signalBuffer) {
			if (signalBuffer > SIGNAL_TICKS_HIGH) signalBuffer = SIGNAL_TICKS_HIGH;
			if (signalBuffer < SIGNAL_TICKS_LOW) signalBuffer = SIGNAL_TICKS_LOW;
			setPWMFromSignal(signalBuffer);
			signalBuffer = 0; //Clear signal
		}
	}
}

void setPWMFromSignal(uint32_t sig)
{
	sig -= SIGNAL_TICKS_LOW;
	OCR2 = ((uint8_t)((sig * PWM_RANGE)/(SIGNAL_TICKS_HIGH - SIGNAL_TICKS_LOW)));
}