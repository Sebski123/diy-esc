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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "config/BLDC_config.h"

//Allows debugging of code via an LED and serial data
#define DEBUG 1 //Set to 1 to allow debugging

//Light an LED for debugging. Choose an unused pin (e.g MISO/MOSI/SCK)
#define LED_DDR			DDRB
#define LED_PORT		PORTB
#define LED				5

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

void init_debug(void);
void setLED(void);
void clrLED(void);
void togLED(void);
void USART_SendChar(char c);
void USART_SendString(char* StringPtr);
void USART_SendInt(int num);
void USART_NewLine(void);

#endif //File guard