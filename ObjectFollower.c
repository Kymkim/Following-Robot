#include "ObjectFollower.h"
/*
	Initializes PE0-2 as ADC Inputs
	
	Priority In Order
	//SS0 = IR Front = PE1
	//SS1 = IR Left = PE0
	//SS2 = IR Right = PE2
	//SS3 = Unused
*/
void IR_Sensor_Init(void){
	
	SYSCTL_RCGCADC_R |= 0x01; //Enable ADC
	
	//PORT E GPIO Initialization
	SYSCTL_RCGCGPIO_R |= 0x10;
	while((SYSCTL_RCGCGPIO_R & 0x10) == 0);
	GPIO_PORTE_DIR_R &= ~0x07;
	GPIO_PORTE_AFSEL_R |= 0x07;
	GPIO_PORTE_DEN_R &= ~0x07;
	GPIO_PORTE_AMSEL_R |= 0x07;
	
	//Sample Rate to 125k sample/sec
	ADC0_PC_R &= ~0xF;
	ADC0_PC_R |= 0x1;
	ADC0_SSPRI_R = 0x3210;
	ADC0_ACTSS_R &= ~0x07;
	ADC0_EMUX_R &= ~0x0FFF;
	
	//PE0 Ain3
	ADC0_SSMUX1_R = (ADC0_SSMUX1_R&0xFFFFFFF0)+3;
	ADC0_SSCTL1_R = 0x0006;  
	//PE1 Ain2
	ADC0_SSMUX0_R = (ADC0_SSMUX0_R&0xFFFFFFF0)+2;
	ADC0_SSCTL0_R = 0x0006;  
	//PE2 Ain0
	ADC0_SSMUX2_R = (ADC0_SSMUX2_R&0xFFFFFFF0)+0;
	ADC0_SSCTL2_R = 0x0006; 
	
	
	ADC0_IM_R &= ~0x07;
	ADC0_ACTSS_R |= 0x07;
	
}

/*
	Data Reading Functions
*/
uint16_t Read_Front(void){
	uint16_t data;
	ADC0_PSSI_R |= 0x1;
	while((ADC0_RIS_R&0x1)==0);
	data = ADC0_SSFIFO0_R&0xFFF;
	return data;
}
uint16_t Read_Left(void){
	uint16_t data;
	ADC0_PSSI_R |= 0x2;
	while((ADC0_RIS_R&0x2)==0);
	data = ADC0_SSFIFO1_R&0xFFF;
	return data;
}	
uint16_t Read_Right(void){
	uint16_t data;
	ADC0_PSSI_R |= 0x4;
	while((ADC0_RIS_R&0x4)==0);
	data = ADC0_SSFIFO2_R&0xFFF;
	return data;
}

/*
	FIR Filters
*/
uint16_t FIR_Front(){
	static uint16_t oldest=0, middle=0;	
  uint16_t newest;
	uint16_t NewValue;
  newest = Read_Front();  // read one value
  NewValue = med(newest, middle, oldest);
  oldest = middle; 
  middle = newest; 
	return NewValue;
}
uint16_t FIR_Left(){
	static uint16_t oldest=0, middle=0;	
  uint16_t newest;
	uint16_t NewValue;
  newest = Read_Left();  // read one value
  NewValue = med(newest, middle, oldest);
  oldest = middle; 
  middle = newest; 
	return NewValue;
}
uint16_t FIR_Right(){
	static uint16_t oldest=0, middle=0;	
  uint16_t newest;
	uint16_t NewValue;
  newest = Read_Right();  // read one value
  NewValue = med(newest, middle, oldest);
  oldest = middle; 
  middle = newest; 
	return NewValue;
}

/*
	Median Function. Helper for FIR Filters
*/
uint16_t med(uint16_t u1, uint16_t u2, uint16_t u3){
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