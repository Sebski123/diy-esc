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

#ifndef __RC_SIGNAL_H__
#define __RC_SIGNAL_H__

//===================================//
//       RC Signal Handling          //
//===================================//
//Converts pulse time measurement to PWM value

//RC servo pulse length should be between LOW and HIGH, in microseconds
#define RCSIGNAL_LOW 	1000
#define RCSIGNAL_HIGH	2000

#define TIMER0_TCCR0		(1 << CS01) //F_CPU/8
#define TIMER0_TIMSK		(1 << TOIE0) //Enable interrupt for timer0
#define uS_PER_TICK0		0.5 //How many microseconds per timer0 tick

#define STOP_TIMER0()		(TCCR0 = 0)
#define START_TIMER0()		(TCCR0 = TIMER0_TCCR0)

//Below is calculating constants based on configuration parameters
#define RCSIGNAL_TICKS_LOW	(RCSIGNAL_LOW / uS_PER_TICK0)
#define RCSIGNAL_TICKS_HIGH	(RCSIGNAL_HIGH / uS_PER_TICK0)

volatile 	uint8_t 	t0_ovfs; //Counts how many times Timer0 has overflowed
volatile	uint16_t	signalBuffer; //Received signal is stored in here; When no signal, this = 0

void init_rc(void);
uint8_t processRCSignal(uint32_t RCsignal);


#endif //File guard