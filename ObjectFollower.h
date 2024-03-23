#include "ADC0SS3.h"  
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include <stdint.h>
#include "Motors.h"

#define LEFT_DIST_FAR 0
#define LEFT_DIST_FOLLOW 0
#define LEFT_DIST_CLOSE 0

#define RIGHT_DIST_FAR 0
#define RIGHT_DIST_FOLLOW 0
#define RIGHT_DIST_CLOSE 0

#define FRONT_DIST_FAR 0
#define FRONT_DIST_FOLLOW 0
#define FRONT_DIST_CLOSE 0

uint16_t delay;

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
uint16_t Read_Front();
uint16_t Read_Left();
uint16_t Read_Right();

/*
	Median Function
*/
uint16_t med(uint16_t u1, uint16_t u2, uint16_t u3);