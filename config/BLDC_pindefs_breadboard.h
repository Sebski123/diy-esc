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
//        ESC PIN DEFINITIONS        //
//===================================//
//Consult microcontroller data sheet before adjusting values
//NOTE: Macro definitions conform to AVR naming conventions.
//	DDR  - Data Direction Register
//	PORT - I/O port name
//	PIN  - Port IN; current state of the I/O port

//---------OUTPUTS----------//
//Channel A
#define HIGH_A_DDR		DDRD
#define HIGH_A_PORT		PORTD
#define HIGH_A			3

#define LOW_A_DDR		DDRD
#define LOW_A_PORT		PORTD
#define LOW_A			4

//Channel B
#define HIGH_B_DDR		DDRD
#define HIGH_B_PORT		PORTD
#define HIGH_B			5

#define LOW_B_DDR		DDRB
#define LOW_B_PORT		PORTB
#define LOW_B			2

//Channel C
#define HIGH_C_DDR		DDRD
#define HIGH_C_PORT		PORTD
#define HIGH_C			7

#define LOW_C_DDR		DDRB
#define LOW_C_PORT		PORTB
#define LOW_C			0

//Light an LED for debugging. Choose an unused pin (e.g MISO/MOSI/SCK)
#define LED_DDR			DDRB
#define LED_PORT		PORTB
#define LED				5

//Are the outputs active low? Inspect circuit diagram to determine.
// 1 = TRUE, 0 = FALSE
#define HIGH_ACTIVE_LOW 0
#define LOW_ACTIVE_LOW 	0

//----------INPUTS----------//
//RC servo signal
#define RC_DDR			DDRD
#define RC_PORT			PORTD
#define RC_PIN			PIND
#define RC				2

//The ADMUX bits needed to select back-EMF ADC input
#define ADC_A			0x01
#define ADC_B			0x02
#define ADC_C			0x03

//Used to read source voltage
#define ADC_VSS			0x00