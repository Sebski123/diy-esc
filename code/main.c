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
#include "config/BLDC_config.h"
//#include "config/BLDC_motor_A2208.h"
//#include "config/BLDC_motor_smallpink.h"
//#include "config/BLDC_motor_smallsilver.h"
#include "config/BLDC_motor_HDD.h"

//Controller is broken up into easy to manage stubs
#include "rc_signal.h"
#include "debug.h"

void setPWM(uint8_t val);
void startMotor(void);
void stopMotor(void);
void nextCommutation(void);
void clrAllOutputs(void);
void init_mosfets(void);
void init_registers(void);

extern volatile	uint16_t 	signalBuffer; //See rc_signal.h

volatile uint8_t 	zcActive; //Whether ZC detection is active or not
volatile uint8_t 	zcACO; //What the ACO reading should be after a ZC
volatile uint8_t 	zcTime; //Timer ticks when ZC was detected
volatile uint8_t	currACO; //used to read ACO output during spike rejection
volatile uint8_t 	t1_comp;
volatile uint8_t 	t1_ovfs;
volatile uint8_t 	startupState;
volatile uint8_t 	pwmVal; //0->255
volatile uint8_t 	pwmPhase; //0=NONE, 1=A, 2=B, 3=C
volatile uint8_t 	motorON; //0=off, 1=on
volatile uint8_t 	commState; //1->6, then loops

int main(void)
{
	
	init_mosfets();
	init_registers();
	init_debug();
	
	switch (INPUT_SIGNAL_MODE) {
	case 1:
		init_rc();
		break;
	case 2:
		//init_twi();
		break;
	case 3:
		//init_uart();
		break;
	default:
		//NO INPUT SELECTED//
		break;
	}
	
	//Initialise variables
	motorON = 0;
	pwmVal = 0;
	
	while(1) {
		if (signalBuffer) { //When signal buffer is populated, update PWM
			pwmVal = processRCSignal(signalBuffer);
			signalBuffer = 0; //Clear signal
			
			if (pwmVal > PWM_ON_THR) {
				startMotor();
				setLED();
				motorON = 1; //Enter 'motor on' loop below
			}
		}
		
		while (motorON) {
			if (signalBuffer) { //When signal buffer is populated, update PWM
				pwmVal = processRCSignal(signalBuffer);
				signalBuffer = 0; //Clear signal
				setPWM(pwmVal);
				
				if (pwmVal < PWM_OFF_THR) {
					//turn off motor (allow slow braking), disable pwm
					TIMER1_STOP();
					PWM_STOP();
					setPWM(0);
					clrAllOutputs();
					clrLED();
					motorON = 0; //Leave 'motor on' loop
				}
			}
		}
	}
}

ISR(TIMER1_CAPT_vect)
{
	zcTime = ICR1;
	OCR1B = zcTime + ZC_NOISE_REJECT;
}

ISR(TIMER1_COMPA_vect)
{
	if (startupState) {
		OCR1A *= (1 - STARTUP_RAMP);
	}
	else {
		TIMSK &= ~(1 << OCIE1A); //disable commutation interrupt
	}
	nextCommutation();
	TCNT1 = 0;
}

ISR(TIMER1_COMPB_vect)
{
	if (zcActive) {
		//Input capture interrupt is set, so zero crossing was detected
		currACO = GET_ACO();
		if (currACO == zcACO) {
			//Analog comparator output has remained the same since interrupt occured, zero crossing has been detected
			ZC_STOP_DETECT();
			zcActive = 0;
			TIFR |= (1 << ICF1); //Make sure input capture flag is cleared by writing logic 1 to it
			
			OCR1A = 2 * zcTime; //Set commutation time to twice the ZC time
			ENABLE_COMM(); //enable commutation to occur
		}
	}
	else {
		//Input capture interrupt is disabled, so blanking period is in process
		ZC_START_DETECT();
		zcActive = 1;
	}
}

ISR(TIMER1_OVF_vect)
{
	t1_ovfs++; //Keep track of how many times timer1 overflows
}

ISR(TIMER2_COMP_vect)
{
	CLR_A_LOW();
	CLR_B_LOW();
	CLR_C_LOW();
}

ISR(TIMER2_OVF_vect)
{
	switch (pwmPhase) {
	case 1:
		SET_A_LOW();
		break;
	case 2:
		SET_B_LOW();
		break;
	case 3:
		SET_C_LOW();
		break;
	}
}

ISR(ANA_COMP_vect)
{
	//PORTB ^= (1 << PB1);
	uint8_t temp = ACSR & (1 << ACO);
	PORTB = ((temp >> ACO) << PB1);
}

void setPWM(uint8_t val)
{
	OCR2 = val;
}

void startMotor(void)
{
	uint8_t i; //used for "for" loops

	//TIMSK &= ~(1 << OCIE1B);
	
	//lock rotor
	pwmPhase = 1;
	setPWM(STARTUP_LOCK_PWM);
	PWM_START(); //enable PWM
	
	TCNT1 = 0;
	commState = 1;
	
	DISABLE_SPIKE_REJECTION();
	DISABLE_COMM();
	TIMER1_START();
	
	for (i=0; i<2; i++) {
		t1_ovfs = 0;
		nextCommutation();
		while (t1_ovfs < STARTUP_RLOCK) {} //wait until defined overflows occurs
	}
	
	//Commutations now handled in interrupts   
	setPWM(STARTUP_RAMP_PWM);                  
	TCNT1 = 0;
	startupState = 1;
	OCR1A = STARTUP_TICKS_BEGIN;
	ENABLE_COMM();
	while (OCR1A > STARTUP_TICKS_END) {} //Wait until ramp has finished
	
	//Getting ready to detect ZC using analog comparator
	DISABLE_COMM();
	startupState = 0;
	zcActive = 0;
	ENABLE_SPIKE_REJECTION();
	OCR1B = ZC_BLANKING_TICKS; //Set blanking period to avoid inductive spikes
}

void stopMotor(void)
{
}

void nextCommutation(void)
{
	//When switching low side, extra logic tests are used to make sure if PWM was in an 'on' state,
	//	it will remain that way and visa versa
	
	//To make code more efficient, it is assumed that commState always starts at 0 after a MCU reset,
	//	and direction of rotation can not change during operation.
	
	//TODO: ADD IF STATEMENT FOR DIRECTION OF SPIN
	
	switch (commState) {

	case 1:
		//Do comparator stuff here before switching occurs
		SET_B_HIGH();
		CLR_C_HIGH();
		ZC_DETECT_FALLING();
		zcACO = 1;
		ADMUX = ADC_C;
		break;
	case 2:
		//Change pwmPhase 1->3, check current state
		if (GET_A_LOW()) {
			SET_C_LOW(); //if previous pwmPhase mosfet was on, set new phase to on
			CLR_A_LOW();
		}
		//If mosfet was off, then the new phase will be activated on the next pwm interrupt,
		//	no need to do anything extra
		pwmPhase = 3; //Set pwm to C_LOW
		ZC_DETECT_RISING();
		zcACO = 0;
		ADMUX = ADC_A;
	  	break;
	case 3:
		SET_A_HIGH();
		CLR_B_HIGH();
		ZC_DETECT_FALLING();
		zcACO = 1;
		ADMUX = ADC_B;
	  	break;
	case 4:
		//Change pwmPhase 3->2, check current state
		if (GET_C_LOW()) {
			SET_B_LOW(); //if previous pwmPhase mosfet was on, set new phase to on
			CLR_C_LOW();
		}
		//If mosfet was off, then the new phase will be activated on the next pwm interrupt,
		//	no need to do anything extra
		pwmPhase = 2; //Set pwm to B_LOW
		ZC_DETECT_RISING();
		zcACO = 0;
		ADMUX = ADC_C;
	  	break;
	case 5:
		SET_C_HIGH();
		CLR_A_HIGH();
		ZC_DETECT_FALLING();
		zcACO = 1;
		ADMUX = ADC_A;
	  	break;
	case 6:
		//Change pwmPhase 2->1, check current state
		if (GET_B_LOW()) {
			SET_A_LOW(); //if previous pwmPhase mosfet was on, set new phase to on
			CLR_B_LOW();
		}
		//If mosfet was off, then the new phase will be activated on the next pwm interrupt,
		//	no need to do anything extra
		pwmPhase = 1; //Set pwm to A_LOW
		commState = 0; //Start from beginning
		ZC_DETECT_RISING();
		zcACO = 0;
		ADMUX = ADC_B;
	  	break;
	}
	commState++;
	
	if (!startupState) OCR1B = ZC_BLANKING_TICKS; //Set blanking period
}

void clrAllOutputs(void)
{
	CLR_A_HIGH();
	CLR_B_HIGH();
	CLR_C_HIGH();
	CLR_A_LOW();
	CLR_B_LOW();
	CLR_C_LOW();
}

void init_mosfets(void)
{
	//Set mosfet drive pins as outputs
    HIGH_A_DDR |= (1 << HIGH_A);
    HIGH_B_DDR |= (1 << HIGH_B);
    HIGH_C_DDR |= (1 << HIGH_C);
    LOW_A_DDR |= (1 << LOW_A);
    LOW_B_DDR |= (1 << LOW_B);
    LOW_C_DDR |= (1 << LOW_C);
		
	//Turn all mosfets off
	clrAllOutputs();
}

void init_registers(void)
{
	//Set up registers, timers and interrupts

	//Edit the following registers in the header file
	
	//Timer1 - commutation timer
	TIMSK |= TIMER1_TIMSK;
	
	//Timer2 - PWM timer
	TCCR2 = TIMER2_TCCR2;
	TIMSK |= TIMER2_TIMSK;
	
	//Analog comparator settings
	SFIOR |= (1 << ACME); //Set comparator -ve input to ADMUX
	ACSR |= (1 << ACIC); //enable comparator to trigger input capture
	
	sei(); //Enable interrupts
}