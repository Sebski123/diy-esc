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
//       ESC SETUP & SETTINGS        //
//===================================//
//NOTE: MCU name and processor frequency are set in the makefile

// 1 = RC Servo Signal on INT0
// 2 = I2C Signal using SDA and SDL (TODO)
// 3 = Serial data on RXD (TODO)
#define INPUT_SIGNAL_MODE 1

//Allows debugging of code via an LED and serial data
#define DEBUG 1
//test

//Change for CW/CCW rotation. Swapping 2 of the motor leads will also reverse direction
#define ROTATION_ORDER 1

//------TIMER1 SETTINGS------//

//Do not change these settings unless you know what they do
//Frequency is determined by CPU speed, timer prescaler and timer compare values

//0-255, 8khz
#define PWM_RANGE		255
#define PWM_MIN			40 //Need to allow MCU to pass bottom/top without missing interrupts
#define PWM_SAFETY		10 //On reset, signal must go below safety before ESC is armed
#if CLIP_SIGNAL == 1
	#define PWM_MAX			230
#else
	#define PWM_MAX			PWM_RANGE
#endif
#define COUNTS_PER_PWM_STEP	2 //How many clock counts per 1 pwm step
#define PWM_TOP			(COUNTS_PER_PWM_STEP * PWM_RANGE)

//Set up Timer1 in clk/1, pwm mode
#define TIMER1_PRESCL	((0 << CS12) | (0 << CS11) | (1 << CS10))
#define TIMER1_REGA		((1 << WGM11) | (0 << COM1A1))
#define TIMER1_REGB		((1 << WGM13) | TIMER1_PRESCL)
#define TIMER1_TIMSK	((1 << OCIE1A) | (0 << OCIE1B) | (1 << TOIE1) | (1 << TICIE1)) //enable interrupts for timer1
#define PWM_ENABLE		(TCCR1B |= TIMER1_PRESCL)
#define PWM_DISABLE		(TCCR1B &= ~TIMER1_PRESCL)


//------TIMER0 SETTINGS------//
//records RC signal length

#if INPUT_SIGNAL_MODE == 1
	//RC servo pulse length should be between LOW and HIGH, in microseconds
	#define SIGNAL_LOW 	1000
	#define SIGNAL_HIGH	2000
	#define TIMER0_REG			(1 << CS01) //clk/8
	#define TIMER0_TIMSK		(1 << TOIE0) //enable interrupt for timer0
	#define uS_PER_TICK0		0.5
	#define SIGNAL_TICKS_LOW	(SIGNAL_LOW / uS_PER_TICK0)
	#define SIGNAL_TICKS_HIGH	(SIGNAL_HIGH / uS_PER_TICK0)
	#if CLIP_SIGNAL == 1
		#define SIGNAL_TICKS_MAX	(SIGNAL_TICKS_LOW + ((SIGNAL_TICKS_HIGH - SIGNAL_TICKS_LOW) / PWM_RANGE) * (PWM_MAX)) //Testing PWM limit
	#else
		#define SIGNAL_TICKS_MAX	SIGNAL_TICKS_HIGH
	#endif
	//5% buffer between on and off thresholds to stop state bouncing between on/off
	#define SIGNAL_TICKS_ONTHR	(SIGNAL_TICKS_LOW + (((SIGNAL_TICKS_HIGH - SIGNAL_TICKS_LOW) / PWM_RANGE) * (PWM_MIN + (PWM_RANGE*0.05)))) //PWM wont turn on until signal is above this value
	#define SIGNAL_TICKS_OFFTHR	(SIGNAL_TICKS_LOW + (((SIGNAL_TICKS_HIGH - SIGNAL_TICKS_LOW) / PWM_RANGE) * PWM_MIN)) //PWM wont turn off until signal is below this value
	#define SIGNAL_TICKS_SAFETY	(SIGNAL_TICKS_LOW + ((SIGNAL_TICKS_HIGH - SIGNAL_TICKS_LOW) * 0.03)) //ESC won't arm until below this value
	
#elif INPUT_SIGNAL_MODE == 2
	
	//TODO: I2C setup here
	
#elif INPUT_SIGNAL_MODE == 3
	
	//TODO: UART setup here
#else
	#error !!! Please select a valid INPUT_SIGNAL_MODE value !!!
	
#endif

//Timer1 settings needed to generate audible beeps from the motor
#define TIMER1_BEEPS_REGA	(1 << WGM11)
#define TIMER1_BEEPS_REGB	((1 << WGM13) | (1 << CS10))

//Top value determines frequency, compare value determines duty cycle (volume)
#define LOWBEEP				7641 //1047Hz (C6)
#define MIDBEEP				6067 //1319Hz (E6)
#define HIGHBEEP			5102 //1568Hz

#define TIMER2_OVFMODE		(TCCR2 &= ~(1 << WGM21)) //normal overflow mode
#define TIMER2_CTCMODE		(TCCR2 |= (1 << WGM21)) //CTC mode
#define TIMER2_START_P64	(TCCR2 |= (1 << CS22)) //start timer at clk/64
#define TIMER2_START_P32	(TCCR2 |= ((1 << CS21) | (1 << CS20))) //start timer at clk/32
#define TIMER2_STOP			(TCCR2 &= ~((1 << CS22) | (1 << CS21) | (1 << CS20))) //stop timer
#define TIMER2_TIMSK		((1 << OCIE2) | (1 << TOIE2)) //enable interrupts for timer2

#define TIMER2_COMM_REG		(1 << CS22) //TCCR2

//Enable all timer interrupts
#define TIMSK_REG			((1 << OCIE2) | (1 << TOIE2) | (1 << OCIE1A) | (0 << OCIE1B) | (1 << TOIE1) | (1 << TICIE1) | (1 << TOIE0))

#define COMPARATOR_ENABLE	(ACSR |= (1 << ACIE))
#define COMPARATOR_DISABLE	(ACSR &= ~(1 << ACIE))


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
	
#elif LOW_ACTIVE_LOW == 1
	//Lowside MOSFET logic is active low
	#define SET_A_LOW() (LOW_A_PORT &= ~(1 << LOW_A))
	#define SET_B_LOW() (LOW_B_PORT &= ~(1 << LOW_B))
	#define SET_C_LOW() (LOW_C_PORT &= ~(1 << LOW_C))
	
	#define CLR_A_LOW() (LOW_A_PORT |= (1 << LOW_A))
	#define CLR_B_LOW() (LOW_B_PORT |= (1 << LOW_B))
	#define CLR_C_LOW() (LOW_C_PORT |= (1 << LOW_C))
	
#else
	#error !!! Please select a valid LOW_ACTIVE_LOW value in pindefs !!!
	
#endif

#if DEBUG == 1
	#define SET_LED() (LED_PORT |= (1 << LED))
	#define CLR_LED() (LED_PORT &= ~(1 << LED))
	#define TOGGLE_LED() (LED_PORT ^= (1 << LED))
#endif

inline void  clrAllOutputs(void){
	//Turn all mosfets off
	CLR_A_HIGH();
	CLR_B_HIGH();
	CLR_C_HIGH();
    CLR_A_LOW();
	CLR_B_LOW();
	CLR_C_LOW();
}

inline void initPins(void){
	//Set up outputs and initialise pins
	
	//Set mosfet drive pins as outputs
    HIGH_A_DDR |= (1 << HIGH_A);
    HIGH_B_DDR |= (1 << HIGH_B);
    HIGH_C_DDR |= (1 << HIGH_C);
    LOW_A_DDR |= (1 << LOW_A);
    LOW_B_DDR |= (1 << LOW_B);
    LOW_C_DDR |= (1 << LOW_C);
	
	if (DEBUG) LED_DDR |= (1 << LED);
	
	//Turn all mosfets off
	clrAllOutputs();
}