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
//     ESC MOTOR CHARACTERISTICS     //
//===================================//
//These values vary per motor, you must determine them through
// inspection and experimentation

//This is only used to calculate current motor RPM
#define COMMUTATIONS_PER_REVOLUTION 42


//TODO: add timing settings for startup

//Startup values, if motor does not start reliably these can be changed
#define STARTUP_LOCK_PWM	0.15*PWM_TOP //PWM during rotor lock
#define STARTUP_RAMP_PWM	0.6*PWM_TOP //Most motors should work with 30%
//All values below assosciated with startup are CLOCK counts, not seconds. Time will be dependant on CPU speed and timer prescaler
#define STARTUP_RLOCK		60 //How many clock overflows to lock the rotor for (1 ovf = 4ms)
#define STARTUP_RAMP		0.003 //(0<x<0.1) The factor which commutation rate will increase every commutation during startup
#define STARTUP_TICKS_BEGIN	65000 //Larger/heavier motors need larger values
#define STARTUP_TICKS_END	10000 //Startup complete once commutation rate is this many clock ticks
								 //If motor is starting but not achieving closed loop control lock, lower this