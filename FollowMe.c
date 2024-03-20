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

#include "ADC0SS3.h"  
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include <stdint.h>
#include "Motors.h"


// move the following constant definitions to ADC0SS3.h
#define TOO_FAR 		(1000)  // replace the zero with the ADC output value for maximum distance
#define FOLLOW_DIST (1500)  // replace the zero with the ADC output value for object following distance
#define TOO_CLOSE 	(3000)  // replace the zero with the ADC output value for minimum distance


void PORTF_Init(void);
uint16_t median(uint16_t u1, uint16_t u2, uint16_t u3);
uint16_t ReadADCMedianFilter(void);
void Delay(void);
void follow_me();

volatile uint16_t ADCvalue;
// The digital number ADCvalue is a representation of the voltage on PE2 
// voltage  ADCvalue
// 0.00V     0
// 0.75V    1024
// 1.50V    2048
// 2.25V    3072
// 3.00V    4095

int main(void){	
  PLL_Init();                         // 16 MHz, 
	PORTF_Init();
	ADC0_InitSWTriggerSeq3_Ch1();     // ADC initialization PE2/AIN1
	//ADC0_InitSWTriggerSeq3_Ch7();       // ADC initialization PD0/AIN7
	Wheels_PWM_Init();
	Dir_Init();
	
	DIRECTION = FORWARD;
	Set_L_Speed(SPEED_35);
	Set_R_Speed(SPEED_35);
	
  while(1){
    follow_me();
  }
	
	
}

// Implement a simple algorithm to follow an object to move forward/backward/stop.
void follow_me(void) {
	uint8_t i;
  volatile uint16_t ADCvalue;
	
	// Calibrate the sensor
	for (i=0;i<10;i++) {
    ADCvalue = ReadADCMedianFilter();  // read one value
	} 
	
  // wait until an obstacle is in the right distant range.
 	do {
			ADCvalue = ReadADCMedianFilter();  // read one value
	} while ((ADCvalue>TOO_CLOSE) || (ADCvalue<TOO_FAR));
		
  while ((ADCvalue<TOO_CLOSE) && (ADCvalue>TOO_FAR)) {								
		ADCvalue = ReadADCMedianFilter();
		
		if (ADCvalue>FOLLOW_DIST + 100) { // negative logic: too close, move back
			DIRECTION = BACKWARD;
			Start_Both_Wheels();
		}
		else if (ADCvalue<FOLLOW_DIST - 50){ // negative logic: too far, move forward
			DIRECTION = FORWARD;
			Start_Both_Wheels();
		}
		else { // right distance, stop
			Stop_Both_Wheels();
		}
  }	
}

void PORTF_Init(void){
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5; // activate port F
	while ((SYSCTL_RCGCGPIO_R&SYSCTL_RCGCGPIO_R5)!=SYSCTL_RCGCGPIO_R5){}
		
  GPIO_PORTF_DIR_R |= 0x04;             // make PF2 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;          // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;             // enable digital I/O on PF2
  GPIO_PORTF_PCTL_R &= ~0x00000F00;     // configure PF2 to be GPIO
  GPIO_PORTF_AMSEL_R &= ~ 0x04;               // disable analog functionality on PF
}

// Median function: 
// A helper function for ReadADCMedianFilter()
uint16_t median(uint16_t u1, uint16_t u2, uint16_t u3){
uint16_t result;
	
  if(u1>u2)
    if(u2>u3)   result=u2;     // u1>u2,u2>u3       u1>u2>u3
      else
        if(u1>u3) result=u3;   // u1>u2,u3>u2,u1>u3 u1>u3>u2
        else      result=u1;   // u1>u2,u3>u2,u3>u1 u3>u1>u2
  else
    if(u3>u2)   result=u2;     // u2>u1,u3>u2       u3>u2>u1
      else
        if(u1>u3) result=u1;   // u2>u1,u2>u3,u1>u3 u2>u1>u3
        else      result=u3;   // u2>u1,u2>u3,u3>u1 u2>u3>u1
  return(result);
}

// This function samples one value, apply a software filter to the value and
// returns filter result.  Some kind of filtering is required because 
// the IR distance sensors output occasional erroneous spikes.  
// This function implements a median filter:
// y(n) = median(x(n), x(n-1), x(n-2))
// Assumes: ADC has already been initialized. 
uint16_t ReadADCMedianFilter(void){
  static uint16_t oldest=0, middle=0;	
  uint16_t newest;
	uint16_t NewValue;
	
  newest = ADC0_InSeq3();  // read one value
  NewValue = median(newest, middle, oldest);
  oldest = middle; 
  middle = newest; 
	return NewValue;
}

void Delay(void) {
	uint32_t delay;
	
  for(delay=0; delay<100000; delay++){};
}