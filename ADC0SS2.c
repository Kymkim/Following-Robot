// ADC0SS2.c
// Runs on TM4C123
// Provide functions that sample ADC inputs PE1, PE4, PE5
// using SS2 to be triggered by software and trigger two
// conversions,wait for them to finish, and return the two
// results.
// Daniel Valvano
// September 12, 2013
// Modified by Min He, 3/16/2024

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
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

#include "tm4c123gh6pm.h"
#include "ADC0SS2.h"
#include <stdint.h>

// There are many choices to make when using the ADC, and many
// different combinations of settings will all do basically the
// same thing.  For simplicity, this function makes some choices
// for you.  When calling this function, be sure that it does
// not conflict with any other software that may be running on
// the microcontroller.  Particularly, ADC0 sample sequencer 2
// is used here because it takes up to four samples, and three
// samples are needed.  Sample sequencer 2 generates a raw
// interrupt when the third conversion is complete, but it is
// not promoted to a controller interrupt.  Software triggers
// the ADC0 conversion and waits for the conversion to finish.
// If somewhat precise periodic measurements are required, the
// software trigger can occur in a periodic interrupt.  This
// approach has the advantage of being simple.  However, it does
// not guarantee real-time.
//
// A better approach would be to use a hardware timer to trigger
// the ADC conversion independently from software and generate
// an interrupt when the conversion is finished.  Then, the
// software can transfer the conversion result to memory and
// process it after all measurements are complete.
#define SYSCTL_RCGCADC_ADC0  0x00000001  // define bit position for activating ADC0 clock

// Initializes AIN2, AIN1, and AIN3 sampling
// 125k max sampling
// SS2 triggering event: software trigger, busy-wait sampling
// SS2 1st sample source: AIN2 (PE1) = Front
// SS2 2nd sample source: AIN1 (PE2) = Right
// SS2 3rd sample source: AIN3 (PE0) = Left
// SS2 interrupts: enabled after 3rd sample but not promoted to controller
void ADC0_SS2_Init213(void){
  volatile uint16_t delay;
  SYSCTL_RCGCADC_R |= 0x00000001; // 1) activate ADC0
	
  //PORT E GPIO Initialization
	SYSCTL_RCGCGPIO_R |= 0x10;
	while((SYSCTL_RCGCGPIO_R & 0x10) == 0);
	GPIO_PORTE_DIR_R &= ~0x07;
	GPIO_PORTE_AFSEL_R |= 0x07;
	GPIO_PORTE_DEN_R &= ~0x07;
	GPIO_PORTE_AMSEL_R |= 0x07;
	
  ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 3 is lowest priority
  ADC0_ACTSS_R &= ~0x0004;        // 10) disable sample sequencer 2
  ADC0_EMUX_R &= ~0x0F00;         // 11) seq2 is software trigger
  ADC0_SSMUX2_R = 0x0312;         // 12) set channels for SS2
  ADC0_SSCTL2_R = 0x0600;         // 13) no D0 END0 IE0 TS0 D1 END1 IE1 TS1 D2 TS2, yes END2 IE2
  ADC0_IM_R &= ~0x0004;           // 14) disable SS2 interrupts
  ADC0_ACTSS_R |= 0x0004;         // 15) enable sample sequencer 2
}

//------------ADC_In498------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: three 12-bit result of ADC conversions
// Samples AIN8, AIN9, and AIN4
// 125k max sampling
// software trigger, busy-wait sampling
// data returned by reference
// Ain2 (PE1) 0 to 4095
// Ain1 (PE2) 0 to 4095
// Ain3 (PE0) 0 to 4095
void ADC0_SS2_In213(uint16_t *ain2, uint16_t *ain1, uint16_t *ain3){
  ADC0_PSSI_R = 0x0004;            // 1) initiate SS2
  while((ADC0_RIS_R&0x04)==0){};   // 2) wait for conversion done:3210->0100->4
  *ain2 = ADC0_SSFIFO2_R&0xFFF;    // 3A) read first result
  *ain1 = ADC0_SSFIFO2_R&0xFFF;    // 3B) read second result
  *ain3 = ADC0_SSFIFO2_R&0xFFF;    // 3C) read third result
  ADC0_ISC_R = 0x0004;             // 4) acknowledge completion
}

// This function samples AIN2 (PE1), AIN1 (PE2), AIN3 (PE0) and
// returns the results in the corresponding variables.  Some
// kind of filtering is required because the IR distance sensors
// output occasional erroneous spikes.  This is an FIR filter:
// y(n) = (x(n) + x(n-1))/2
// Assumes: ADC initialized by previously calling ADC_Init298()
void ReadADCFIRFilter(uint16_t *ain2, uint16_t *ain1, uint16_t *ain3){
  //                   x(n-1)
  static uint16_t ain2previous=0; // after the first call, the value changed to 12
  static uint16_t ain1previous=0;
  static uint16_t ain3previous=0;
  // save some memory; these do not need to be 'static'
  //            x(n)
  uint16_t ain2newest;
  uint16_t ain1newest;
  uint16_t ain3newest;
  ADC0_SS2_In213(&ain2newest, &ain1newest, &ain3newest);
  *ain2 = (ain2newest + ain2previous)/2;
  *ain1 = (ain1newest + ain1previous)/2;
  *ain3 = (ain3newest + ain3previous)/2;
  ain2previous = ain2newest; ain1previous = ain1newest; ain3previous = ain3newest;
}

// This function samples AIN4 (PD3), AIN9 (PE4), AIN8 (PE5) and
// returns the results in the corresponding variables.  Some
// kind of filtering is required because the IR distance sensors
// output occasional erroneous spikes.  This is an IIR filter:
// y(n) = (x(n) + y(n-1))/2
// Assumes: ADC initialized by previously calling ADC_Init298()
void ReadADCIIRFilter(uint16_t *ain2, uint16_t *ain1, uint16_t *ain3){
  //                   y(n-1)
  static uint16_t filter2previous=0;
  static uint16_t filter1previous=0;
  static uint16_t filter3previous=0;
  // save some memory; these do not need to be 'static'
  //            x(n)
  uint16_t ain2newest;
  uint16_t ain1newest;
  uint16_t ain3newest;
  ADC0_SS2_In213(&ain2newest, &ain1newest, &ain3newest); // sample AIN2(PE1), AIN9 (PE4), AIN8 (PE5)
  *ain2 = filter2previous = (ain2newest + filter2previous)/2;
  *ain1 = filter1previous = (ain1newest + filter1previous)/2;
  *ain3 = filter3previous = (ain3newest + filter3previous)/2;
}

// Median function from EE345M Lab 7 2011; Program 5.1 from Volume 3
// helper function for ReadADCMedianFilter() but works for general use
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
// This function samples AIN4 (PD3), AIN9 (PE4), AIN8 (PE5) and
// returns the results in the corresponding variables.  Some
// kind of filtering is required because the IR distance sensors
// output occasional erroneous spikes.  This is a median filter:
// y(n) = median(x(n), x(n-1), x(n-2))
// Assumes: ADC initialized by previously calling ADC_Init298()
void ReadADCMedianFilter(uint16_t *ain2, uint16_t *ain1, uint16_t *ain3){
  static uint16_t ain2oldest=0, ain2middle=0;
  static uint16_t ain1oldest=0, ain1middle=0;
  static uint16_t ain3oldest=0, ain3middle=0;
  uint16_t ain2newest;
  uint16_t ain1newest;
  uint16_t ain3newest;
	
  ADC0_SS2_In213(&ain2newest, &ain1newest, &ain3newest); // sample AIN2(PE1), AIN9 (PE4), AIN8 (PE5)
  *ain2 = median(ain2newest, ain2middle, ain2oldest);
  *ain1 = median(ain1newest, ain1middle, ain1oldest);
  *ain3 = median(ain3newest, ain3middle, ain3oldest);
  ain2oldest = ain2middle; ain1oldest = ain1middle; ain3oldest = ain3middle;
  ain2middle = ain2newest; ain1middle = ain1newest; ain3middle = ain3newest;
}
