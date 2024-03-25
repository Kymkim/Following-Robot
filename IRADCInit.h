#include "tm4c123gh6pm.h"
#include "PLL.h"
#include <stdint.h>
#include "Motors.h"

/*
	Initializes PE0-2 as ADC Inputs
*/
void IR_Sensor_Init(void);

/*
	Object Following Algorithm
*/
void Object_Follow(void);

/*
	Read Functions
*/
uint16_t Read_Front();
uint16_t Read_Left();
uint16_t Read_Right();

/*
	Filter Functions
*/
uint16_t FIR_Front();
uint16_t FIR_Left();
uint16_t FIR_Right();

/*
	Median Function
*/
uint16_t med(uint16_t u1, uint16_t u2, uint16_t u3);