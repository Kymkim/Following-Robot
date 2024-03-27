// PWM.h
// Runs on TM4C123
// Use PWM0/PB6 and PWM1/PB7 to generate pulse-width modulated outputs.
// Daniel Valvano
// March 28, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
  Program 6.7, section 6.3.2

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// Modified by Min He, September 7, 2021
#include <stdint.h>

/*
 New Pinout
 Left
 PWM PB4
 DIR PB2
 SLP PB3
 
 Right
 PWM PB5
 DIR PB6
 SLP PB7
*/

#define DIRECTION (*((volatile unsigned long *)0x40005330))
#define FORWARD 			0xCC	//1100 1100, both wheels move forward
#define BACKWARD 			0x88	//0100 0100, both wheels move backward
#define LEFTPIVOT   	0x8C
#define RIGHTPIVOT  	0xC8 

#define LIGHT (*((volatile unsigned long *)0x40025038)) // onboard RBG LEDs are used to show car status. 
#define RED 0x02
#define GREEN 0x08
#define BLUE 0x04

// duty cycles for different speeds
#define STOP 1
#define SPEED_35 3500
#define SPEED_60 6000
#define SPEED_80 8000
#define SPEED_98 9800

// Wheel PWM connections: on PB6/M0PWM0:Left wheel, PB7/M0PWM0:Right wheel
void Wheels_PWM_Init(void);

// Start left wheel
void Start_L(void);

// Start right wheel
void Start_R(void);

// Stop left wheel
void Stop_L(void);

// Stop right wheel
void Stop_R(void);

void Start_Both_Wheels(void);

void Stop_Both_Wheels(void);

void Move_Forward(void);
void Move_Backward(void);
void Move_Left_Pivot(void);
void Move_Right_Pivot(void);
void Move_Left_Forward(void);
void Move_Right_Forward(void);
void Move_Left_Backward(void);
void Move_Right_Backward(void);

void Move_Forward_Follower(void);

void Move_Backward_Follower(void);

void Move_Right_Forward_Follower(void);

void Move_Left_Forward_Follower(void);


// Change duty cycle of left wheel: PB6
void Set_L_Speed(uint16_t duty);

// change duty cycle of right wheel: PB7
void Set_R_Speed(uint16_t duty);

// Initialize port E pins PE0-3 for output
// PE0-3 control directions of the two motors: PE3210:L/SLP,L/DIR,R/SLP,R/DIR
// Inputs: None
// Outputs: None
void Dir_Init(void);

