// ADCTestMain.c
// Runs TM4C123
// This program periodically samples ADC channel 1 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// October 20, 2013
// Modified by Min He, 10/9/2022

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

#include "IRADCInit.h"  
#include "ObjectFollower.h"
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include <stdint.h>
#include "Motors.h"

#define LEFT_DIST_FAR 1500
#define LEFT_DIST_FOLLOW 2400
#define LEFT_DIST_CLOSE 2900

#define RIGHT_DIST_FAR 1500
#define RIGHT_DIST_FOLLOW 2400
#define RIGHT_DIST_CLOSE 2900

#define FRONT_DIST_FAR 1500
#define FRONT_DIST_FOLLOW 2400
#define FRONT_DIST_CLOSE 2800

void object_follower();

int main(void){
	//ADC Initialization
  PLL_Init();                         // 16 MHz, 
	IR_Sensor_Init();
	
	//Wheel Initialization
	Wheels_PWM_Init();
	Dir_Init();
	DIRECTION = FORWARD;
	Set_L_Speed(SPEED_35);
	Set_R_Speed(SPEED_35);
	
	volatile uint16_t LEFT_IR, RIGHT_IR, FRONT_IR;
	
	while(1){
		object_follower();
	}
}

void object_follower(){
	uint8_t i;
  volatile uint16_t LEFT_IR, RIGHT_IR, FRONT_IR;
	
	// Calibrate the sensor
	for (i=0;i<10;i++) {
    LEFT_IR = FIR_Left();
		RIGHT_IR = FIR_Right();
		FRONT_IR = FIR_Front();
	} 
	
  // wait until an obstacle is in the right distant range.
 	do {
		LEFT_IR = FIR_Left();
		RIGHT_IR = FIR_Right();
		FRONT_IR = FIR_Front();
	} while (
		(FRONT_IR>FRONT_DIST_CLOSE)	||	
		(FRONT_IR<FRONT_DIST_FAR)	|| 
		(LEFT_IR>LEFT_DIST_CLOSE) || 
		(LEFT_IR<LEFT_DIST_FAR)	|| 
		(RIGHT_IR>RIGHT_DIST_CLOSE)	||	
		(RIGHT_IR<FRONT_DIST_FAR)
	);
		
  while (
		((FRONT_IR<FRONT_DIST_CLOSE)	&&		(FRONT_IR>FRONT_DIST_CLOSE))	|| 
		((LEFT_IR<LEFT_DIST_CLOSE)		&&		(LEFT_IR>LEFT_DIST_CLOSE))		|| 
		((RIGHT_IR<RIGHT_DIST_CLOSE)	&& 	(RIGHT_IR>RIGHT_DIST_CLOSE)) 
	) 
	{								
		LEFT_IR = FIR_Left();
		RIGHT_IR = FIR_Right();
		FRONT_IR = FIR_Front();
		
		if (FRONT_IR>FRONT_DIST_FOLLOW) { // negative logic: too close, move back
			DIRECTION = BACKWARD;
			Start_Both_Wheels();
		}
		else if (FRONT_IR<FRONT_DIST_FOLLOW) { // negative logic: too far, move forward
			DIRECTION = FORWARD;
			/*
			if(LEFT_IR < RIGHT_IR){
				Set_L_Speed(SPEED_35); 
        Set_R_Speed(SPEED_60);
			}
			else if (RIGHT_IR < LEFT_IR) {
				Set_L_Speed(SPEED_60); 
        Set_R_Speed(SPEED_35);				
			} else {
        Set_L_Speed(SPEED_60); 
        Set_R_Speed(SPEED_60);
			}
			*/
			Start_Both_Wheels();
		}else{
			Stop_Both_Wheels();
		}
  }	
}

void wall_follower(){
	uint8_t i;
  volatile uint16_t LEFT_IR, RIGHT_IR, FRONT_IR;
	
	// Calibrate the sensor
	for (i=0;i<10;i++) {
    LEFT_IR = FIR_Left();
		RIGHT_IR = FIR_Right();
		FRONT_IR = FIR_Front();
	} 
	
	do {
		LEFT_IR = FIR_Left();
		RIGHT_IR = FIR_Right();
		FRONT_IR = FIR_Front();
	} while (
		((FRONT_IR>FRONT_DIST_CLOSE)	||	(FRONT_IR<FRONT_DIST_FAR))	&&
		((LEFT_IR>LEFT_DIST_CLOSE)		||	(LEFT_IR<LEFT_DIST_FAR))
	);
	
	while (
		((FRONT_IR<FRONT_DIST_CLOSE)	&&		(FRONT_IR>FRONT_DIST_CLOSE))	|| 
		((LEFT_IR<LEFT_DIST_CLOSE)		&&		(LEFT_IR>LEFT_DIST_CLOSE))		|| 
		((RIGHT_IR<RIGHT_DIST_CLOSE)	&& 	(RIGHT_IR>RIGHT_DIST_CLOSE)) 
	){
	}
}



