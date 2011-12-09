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

#ifndef __BLDC_CONFIG_H__
#define __BLDC_CONFIG_H__

//===================================//
//       ESC SETUP & SETTINGS        //
//===================================//
//NOTE: MCU name and processor frequency are set in the makefile

#include <avr/io.h>

// 1 = RC Servo Signal on INT0
// 2 = I2C Signal using SDA and SDL (TODO)
// 3 = Serial data on RXD (TODO)
#define INPUT_SIGNAL_MODE 	1

//Change for CW/CCW rotation. Swapping 2 of the motor leads will also reverse direction
#define ROTATION_ORDER 		1

//8-bit (0-255), 7.8khz
#define PWM_TOP				255 //Maximum pwmVal possible
//Leave a buffer between on and off (on > off) to avoid switching rapidly between on/off state
#define PWM_ON_THR			20 //When off, pwmVal must go above this value before motor will turn on
#define PWM_OFF_THR			10 //When running, motor will turn off when pwmVal goes below this value

//TIMER 1 - Commutation Timing
//TODO
#define TIMER1_TIMSK		(1 << TOIE1) //Enable interrupts for timer1
#define TIMER1_PRESCL		(1 << CS10)
#define TIMER1_START()		(TCCR1B |= TIMER1_PRESCL) //Start timer at defined prescaler
#define TIMER1_STOP()		(TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10))) //Clear all prescaler bits (stop the timer)
#define DISABLE_COMM()		(TIMSK &= ~(1 << OCIE1A)) //Disable Timer1 OCR1A interrupt
#define ENABLE_COMM()		(TIMSK |= (1 << OCIE1A)) //Enable Timer1 OCR1A interrupt
#define DISABLE_SPIKE_REJECTION() (TIMSK &= ~(1 << OCIE1B)) //Disable Timer1 OCR1B interrupt
#define ENABLE_SPIKE_REJECTION()  (TIMSK |= (1 << OCIE1B)) //Enable Timer1 OCR1B interrupt

//TIMER 2 - PWM Timing
#define TIMER2_TIMSK		((1 << OCIE2) | (1 << TOIE2)) //Enable interrupts for timer2
#define TIMER2_TCCR2		((1 << WGM21) | (1 << WGM20)) //Fast PWM mode
#define TIMER2_PRESCL		((0 << CS22) | (1 << CS21) | (0 << CS20)) //clk/8
#define PWM_START()			(TCCR2 |= TIMER2_PRESCL) //Start timer at designated prescaler
#define PWM_STOP()			(TCCR2 &= ~TIMER2_PRESCL) //Stop timer

//Top value determines frequency, compare value determines duty cycle (volume)
#define LOWBEEP				//TODO
#define MIDBEEP				//*
#define HIGHBEEP			//*


#if INPUT_SIGNAL_MODE == 2
	
	//TODO: I2C setup here
	
#elif INPUT_SIGNAL_MODE == 3
	
	//TODO: UART setup here
#endif

#define GET_ACO()			((ACSR & (1 << ACO)) >> ACO) //analog comparator reading shifted to bit 1

#define ZC_START_DETECT()	(TIMSK |= (1 << TICIE1)) // enable/disable input capture interrupt
#define ZC_STOP_DETECT()	(TIMSK &= ~(1 << TICIE1))

#define ZC_DETECT_FALLING()	(TCCR1B |= (1 << ICES1)) //A rising comparator is actually a falling B-EMF
#define ZC_DETECT_RISING()	(TCCR1B &= ~(1 << ICES1)) //A falling comparator is actually a rising B-EMF

#define ZC_BLANKING_TICKS	200 //Timer1 ticks to wait before enabling zero cross detection. Needs to be ~12uS
#define ZC_NOISE_REJECT		200 //ZC detection will reject spikes less than this value of clock ticks


//------OUTPUT PIN STATES------//

//Predefine macro functions for mosfet states based on whether logic is active high or low.

#if HIGH_ACTIVE_LOW == 0
	//Highside MOSFET logic is active high
	#define SET_A_HIGH() (HIGH_A_PORT |= (1 << HIGH_A))
	#define SET_B_HIGH() (HIGH_B_PORT |= (1 << HIGH_B))
	#define SET_C_HIGH() (HIGH_C_PORT |= (1 << HIGH_C))
	
	#define CLR_A_HIGH() (HIGH_A_PORT &= ~(1 << HIGH_A))
	#define CLR_B_HIGH() (HIGH_B_PORT &= ~(1 << HIGH_B))
	#define CLR_C_HIGH() (HIGH_C_PORT &= ~(1 << HIGH_C))
	
#elif HIGH_ACTIVE_LOW == 1
	//Highside MOSFET logic is active low
	#define SET_A_HIGH() (HIGH_A_PORT &= ~(1 << HIGH_A))
	#define SET_B_HIGH() (HIGH_B_PORT &= ~(1 << HIGH_B))
	#define SET_C_HIGH() (HIGH_C_PORT &= ~(1 << HIGH_C))
	
	#define CLR_A_HIGH() (HIGH_A_PORT |= (1 << HIGH_A))
	#define CLR_B_HIGH() (HIGH_B_PORT |= (1 << HIGH_B))
	#define CLR_C_HIGH() (HIGH_C_PORT |= (1 << HIGH_C))
	
#else
	#error !!! Please select a valid HIGH_ACTIVE_LOW value in pindefs !!!
	
#endif

#if LOW_ACTIVE_LOW == 0
	//Lowside MOSFET logic is active high
	#define SET_A_LOW() (LOW_A_PORT |= (1 << LOW_A))
	#define SET_B_LOW() (LOW_B_PORT |= (1 << LOW_B))
	#define SET_C_LOW() (LOW_C_PORT |= (1 << LOW_C))
	
	#define CLR_A_LOW() (LOW_A_PORT &= ~(1 << LOW_A))
	#define CLR_B_LOW() (LOW_B_PORT &= ~(1 << LOW_B))
	#define CLR_C_LOW() (LOW_C_PORT &= ~(1 << LOW_C))
	
	//A test to determine whether the lowside mosfets are on/off
	#define GET_A_LOW() ((LOW_A_PIN & (1 << LOW_A)) ? 1 : 0)
	#define GET_B_LOW() ((LOW_B_PIN & (1 << LOW_B)) ? 1 : 0)
	#define GET_C_LOW() ((LOW_C_PIN & (1 << LOW_C)) ? 1 : 0)
	
#elif LOW_ACTIVE_LOW == 1
	//Lowside MOSFET logic is active low
	#define SET_A_LOW() (LOW_A_PORT &= ~(1 << LOW_A))
	#define SET_B_LOW() (LOW_B_PORT &= ~(1 << LOW_B))
	#define SET_C_LOW() (LOW_C_PORT &= ~(1 << LOW_C))
	
	#define CLR_A_LOW() (LOW_A_PORT |= (1 << LOW_A))
	#define CLR_B_LOW() (LOW_B_PORT |= (1 << LOW_B))
	#define CLR_C_LOW() (LOW_C_PORT |= (1 << LOW_C))
	
	//A test to determine whether the lowside mosfets are on/off
	#define GET_A_LOW() ((LOW_A_PIN & (1 << LOW_A)) ? 0 : 1)
	#define GET_B_LOW() ((LOW_B_PIN & (1 << LOW_B)) ? 0 : 1)
	#define GET_C_LOW() ((LOW_C_PIN & (1 << LOW_C)) ? 0 : 1)
	
#else
	#error !!! Please select a valid LOW_ACTIVE_LOW value in pindefs !!!
	
#endif

#endif //File guard