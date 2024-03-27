// SpaceExplorer.c.c
// Runs TM4C123
// Test program for ADC multiple samples. Romi chassis is used in this project.
// ADC0 sequencer 2 is used to connect to analog channels: AIN4 (PD3), AIN9 (PE4), AIN8 (PE5)
// Wheel PWM connections: on PB6/M0PWM0:Left wheel, PB7/M0PWM0:Right wheel
// PE0-3 control directions of the two motors: PE3210:L/SLP,L/DIR,R/SLP,R/DIR
// by Min He, 03/17/2024

#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>
#include "ADC0SS2.h"  
#include "Motors.h"
#include "PLL.h"

void object_steering(uint16_t ahead, uint16_t left, uint16_t right);
void wall_steering(uint16_t ahead, uint16_t left, uint16_t right);
void SwitchLED_Init(void);

extern void DisableInterrupts(void); // Disable interrupts
extern void EnableInterrupts(void);  // Enable interrupts
extern void WaitForInterrupt(void);  // low power mode

int mode, active; //1 Object Follower, 2 Left Wall Follower, 3 Right Wall Follower
uint16_t global_left, global_right, global_ahead;

int main(void){	
	
  PLL_Init();               // set system clock to 16 MHz 
	ADC0_SS2_Init213();       // Initialize ADC0 Sample sequencer 2 to AIN4 (PD3), AIN9 (PE4), AIN8 (PE5)
	Wheels_PWM_Init();
	Dir_Init();
	Set_L_Speed(SPEED_98);
	Set_R_Speed(SPEED_98);
	

  // calibrate the sensors
	for (uint8_t i=0;i<10;i++) {
			ReadADCMedianFilter(&global_ahead, &global_right, &global_left);
	}	
	
	EnableInterrupts();
	SwitchLED_Init();
	
	LIGHT = RED;
	
	mode = 1;
	active = 0;

  while(1){
		ReadADCMedianFilter(&global_ahead, &global_right, &global_left);
		object_steering(global_ahead, global_right, global_left);
  }
}

// Simple steering function to help students get started with project 2.
void object_steering(uint16_t ahead, uint16_t right, uint16_t left){
	if (active == 1){
		if (mode==1){
			LIGHT = BLUE;
			if ((ahead > STOP_DIST)||(left > STOP_DIST)||(right > STOP_DIST)) {
				Stop_Both_Wheels();
				while(ahead > FOLLOW_DIST + 200){
					Move_Backward();
					return;
				}
			}
			if (ahead < FOLLOW_DIST) { //Object Nearby. Follow Object
				DIRECTION = FORWARD;
				if (left < FOLLOW_DIST){  // right side is closer to an object
					Set_R_Speed(SPEED_35);
					Start_R();
				}else{
					Stop_R();
				}
				if (right < FOLLOW_DIST){
					Set_L_Speed(SPEED_35);
					Start_L(); // left side is closer to an object
				}else{
					Stop_L();
				}
				return;
			}
		}
		if ((mode == 2 ) || (mode == 3)){
			LIGHT = GREEN;
			if (mode == 2){ //Left Wall Follower
				if (left < WALL_DIST){ //If none on left side
					Move_Left_Forward();
					return;
				}
				if (ahead > WALL_DIST){ //If wall ahead
					Move_Right_Pivot();
					return;
				}
			}else if (mode == 3){ //Right Wall Follower
				if  (right < WALL_DIST){ //If none on left side
					Move_Right_Forward();
					return;
				}
				if (ahead > WALL_DIST){ //If wall ahead
					Move_Left_Pivot();
					return;
				}
		}
		Move_Forward();
		}
	}else{
		Stop_Both_Wheels();
		LIGHT = RED;
	}
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

void GPIOPortF_Handler(void){
	if(GPIO_PORTF_RIS_R&0x01){ //Mode toggle
		GPIO_PORTF_ICR_R = 0x01;
		if (active == 1){ //If only active
			if(mode == 1){
				if (global_left > global_right){ //Left Mode
					mode = 2;
				}else{
					mode = 3;
				}
			}else{
				mode = 1;
			}
		}
	}else{ //Activation
		GPIO_PORTF_ICR_R = 0x10;
		if (active == 1){
			active = 0;
		}else{
			active = 1;
		}
	}
}

