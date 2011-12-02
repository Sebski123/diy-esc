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
//       RC Signal Handling          //
//===================================//
//Converts pulse time measurement to PWM value

#include <avr/io.h>
#include <avr/interrupt.h>

#include "config/BLDC_pindefs_breadboard.h"
#include "config/BLDC_motor_A2208.h"
#include "config/BLDC_config.h"

#include "rc_signal.h"

ISR(INT0_vect) //Signal capture
{
	STOP_TIMER0(); //Stop timer at an edge detection to ensure accurate pulse timing

	if (RC_PIN & (1<<RC)) {
		//Pin is high which means the signal is on the rising edge (start of pulse)
		//Reset timer count and overflows, and start timer
		
		TCNT0 = 0;
		t0_ovfs = 0;
		START_TIMER0(); //Start timer
	}
	else {
		//Pin is low which means the signal is on the falling edge (end of pulse)
		//Record time since start of pulse, and store value in signalBuffer
		
		if (TIFR & (1<<TOV0)) t0_ovfs++; //Catches when the timer has overflowed since start of this interrupt vector
		signalBuffer = ((t0_ovfs << 8) | TCNT0); //Combine overflows and counts into one 16 bit var
		
		//No need to time the off-pulse, and the timer is already stopped, so timer is not reset until rising edge
	}

}

ISR(TIMER0_OVF_vect)
{
	//Timer for the RC servo signal capture, just increments an overflow variable
	// which creates a virtual 16-bit timer
	t0_ovfs++;
}

void init_rc(void)
{
	//INT0 pin initialisation
	GICR = (1<<INT0); //Enable external interrupt INT0
	MCUCR = (1<<ISC00); //Set INT0 to detect logic toggle
	
	//Timer0 initialisation
	TIMSK |= TIMER0_TIMSK; //Enable Timer0 interrupts
	TCCR0 = TIMER0_TCCR0; //Configure mode and prescaler
}