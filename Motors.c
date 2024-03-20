// PWM.c
// Runs on TM4C123
// Use PWM0/PB6 and PWM1/PB7 to generate pulse-width modulated outputs.
// Daniel Valvano
// March 28, 2014
// Modified by Min He, September 7, 2021

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
 
 
 /*
 Original Pinout
 Left:
 PWM PB6
 DIR PE0
 SLP PE1
 
 Right:
 PWM PB7
 DIR PE2
 SLP PE3
 
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
 
#include <stdint.h>
#include "Motors.h"
#include "tm4c123gh6pm.h"

#define PERIOD 10000				// Total PWM period
#define PLL_SYSDIV_50MHZ 7

uint16_t curr_speed_idx = 0;
uint16_t speeds[] = {STOP, SPEED_35, SPEED_60, SPEED_80, SPEED_98};
#define NUM_OF_SPEEDS		5


// Wheel PWM connections: on PB4/M0PWM2:Left wheel, PB5/M0PWM3:Right wheel
void Wheels_PWM_Init(void){
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x02;            // 2) activate port B: 000010
  while((SYSCTL_RCGCGPIO_R&0x02) == 0){};
	GPIO_PORTB_AFSEL_R |= 0x30;           // enable alt funct on PB4 and PB5: 0011 0000
  GPIO_PORTB_PCTL_R &= ~0x00FF0000;     // configure PB4 and PB5 as PWM0
  GPIO_PORTB_PCTL_R |= 0x00440000;
  GPIO_PORTB_AMSEL_R &= ~0x30;          // disable analog functionality on PB6
  GPIO_PORTB_DEN_R |= 0x30;             // enable digital I/O on PB6
  GPIO_PORTB_DR8R_R |= 0x30;    // enable 8 mA drive on PB6,7
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
    (SYSCTL_RCC_R & (~0x001E0000));   //    configure for /2 divider: PWM clock: 80Mhz/2=40MHz
  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // 3) use PWM divider
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; //    clear PWM divider field
  SYSCTL_RCC_R |= SYSCTL_RCC_PWMDIV_2;  //    configure for /2 divider

	PWM0_1_CTL_R = 0;                     // 4) re-loading down-counting mode
	PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAD_ONE|PWM_1_GENA_ACTLOAD_ZERO;   // PB6: low on LOAD, high on CMPA down
	PWM0_1_GENB_R = (PWM_1_GENB_ACTCMPBD_ONE|PWM_1_GENB_ACTLOAD_ZERO); // PB7: 0xC08: low on LOAD, high on CMPB down
  PWM0_1_LOAD_R = PERIOD - 1;           // 5) cycles needed to count down to 0
  PWM0_1_CTL_R |= 0x00000001;           // 7) start PWM0
}

// Start left wheel
void Start_L(void) {
  PWM0_ENABLE_R |= 0x00000004;          // PB6/M0PWM0
}

// Start right wheel
void Start_R(void) {
  PWM0_ENABLE_R |= 0x00000008;          // enable PB4/M0PWM2
}

// Stop left wheel
void Stop_L(void) {
  PWM0_ENABLE_R &= ~0x00000004;          // PB6/M0PWM0
}

// Stop right wheel
void Stop_R(void) {
  PWM0_ENABLE_R &= ~0x00000008;          // enable PB7/M0PWM1
}

void Start_Both_Wheels(void){
	Start_R();
	Start_L();
}

void Stop_Both_Wheels(void) {
	Stop_L();
	Stop_R();
}
// Set duty cycle for Left Wheel: PB6
void Set_L_Speed(uint16_t duty){
  PWM0_1_CMPA_R = duty - 1;             // 6) count value when output rises
}
// Set duty cycle for Right Wheel: PB7
void Set_R_Speed(uint16_t duty){
  PWM0_1_CMPB_R = duty - 1;             // 6) count value when output rises
}

// Initialize port E pins PE0-3 for output
// PE0-3 control directions of the two motors: PE3210:L/SLP,L/DIR,R/SLP,R/DIR
// Inputs: None
// Outputs: None
void Dir_Init(void){
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; //activate E clock
	while ((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOB)!= SYSCTL_RCGC2_GPIOB){} //wait for clk
	
	
	
	GPIO_PORTB_AMSEL_R &= ~0xCC; //disable analog function
	GPIO_PORTB_PCTL_R &= ~0xFF00FF00; //GPIO clear bit PCTL
	GPIO_PORTB_DIR_R |= 0xCC; //PE0-3 output
	GPIO_PORTB_AFSEL_R &= ~0xCC; //no alternate function
	GPIO_PORTB_DEN_R |= 0xCC; //enable digital pins PE0-3
}

// Initilize port F and arm PF4, PF0 for falling edge interrupts
void SwitchLED_Init(void){  
	unsigned long volatile delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // (a) activate clock for port F
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; // unlock GPIO Port F
  GPIO_PORTF_CR_R |= 0x1F;         // allow changes to PF4,0
  GPIO_PORTF_DIR_R &= ~0x11;    // (c) make PF4,0 in (built-in button)
  GPIO_PORTF_DIR_R |= 0x0E;     // make PF1-3 out
  GPIO_PORTF_AFSEL_R &= ~0x1F;  //     disable alt funct on PF4,0
  GPIO_PORTF_DEN_R |= 0x1F;     //     enable digital I/O on PF4,0
  GPIO_PORTF_PCTL_R &= ~0x000FFFFF; //  configure PF4,0 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x1F;  //     disable analog functionality on PF4,0
  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R |= 0x11;    //     PF4,PF0 rising edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF1FFFFF)|0x00400000; // (g) bits:23-21 for PORTF, set priority to 5
  NVIC_EN0_R |= 0x40000000;      // (h) enable interrupt 30 in NVIC
  GPIO_PORTF_DATA_R &= ~0x0E;   	// make PF1-3 low
	GPIO_PORTF_DATA_R |= RED; 		//make LED red
}

// PORTF ISR:
// Change speed and direction of the two wheels: 
// sw1: move to the next speed in speed array
// sw2: inverse direction
void GPIOPortF_Handler(void){ // called on touch of either SW1 or SW2
	uint32_t t = 727240*20/91; // 20ms to debounce the button
	while(t) t--;
	
  if(GPIO_PORTF_RIS_R&0x01){  // SW2 touched 
    GPIO_PORTF_ICR_R = 0x01;  // acknowledge flag0
		if(DIRECTION == FORWARD){
			DIRECTION = BACKWARD;
			if(speeds[curr_speed_idx] != STOP) GPIO_PORTF_DATA_R = BLUE; 			//make LED blue
		} else if (DIRECTION == BACKWARD){
			DIRECTION = FORWARD;
			if(speeds[curr_speed_idx] != STOP) GPIO_PORTF_DATA_R = GREEN; 		//make LED green
		}
  }
	
  if(GPIO_PORTF_RIS_R&0x10){  // SW1 touch
    GPIO_PORTF_ICR_R = 0x10;  // acknowledge flag4
		curr_speed_idx = (curr_speed_idx+1)%NUM_OF_SPEEDS;
		
		if(curr_speed_idx != 0){					//update led color
			if(DIRECTION == FORWARD)
				GPIO_PORTF_DATA_R = GREEN;
			if(DIRECTION == BACKWARD)
				GPIO_PORTF_DATA_R = BLUE;
		} else 
			GPIO_PORTF_DATA_R = RED;
		
		Set_L_Speed(speeds[curr_speed_idx]);				// update duty cycle
		Set_R_Speed(speeds[curr_speed_idx]);
  }
}



