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
//         DEBUG HANDLING            //
//===================================//
//Everything needed to use the debug LED and send USART data

#include "debug.h"

void init_debug(void)
{
	if (DEBUG) {
		//Set LED as output if debug flag is set
		LED_DDR |= (1 << LED);
		
		//Set USART registers for transmitting data
		UCSRB |= (1 << TXEN);
		UBRRH = (BAUD_PRESCALE >> 8);
		UBRRL = BAUD_PRESCALE;
	}
}

void setLED(void)
{
	if (DEBUG) {
		LED_PORT |= (1 << LED);
	}
}

void clrLED(void)
{
	if (DEBUG) {
		LED_PORT &= ~(1 << LED);
	}
}

void togLED(void)
{
	if (DEBUG) {
		LED_PORT ^= (1 << LED);
	}
}

void USART_SendChar(char c)
{
	if (DEBUG) {
		while (!(UCSRA & (1 << UDRE))); // Do nothing until UDR is ready for more data to be written to it 
		UDR = c;
	}
}

void USART_SendString(char* StringPtr)
{
	if (DEBUG) {
		while (*StringPtr)
		{
			USART_SendChar(*StringPtr);
			StringPtr++;
		}
	}
}

void USART_SendInt(int num)
{
	if (DEBUG) {
		char str[6];
		itoa(num,str,10);
		USART_SendString(str);
	}
}

void USART_NewLine(void)
{
	if (DEBUG) {
		while (!(UCSRA & (1 << UDRE))); // Do nothing until UDR is ready for more data to be written to it
		UDR = 10; //Ascii 10 = LF, 13 = CR
	}
}