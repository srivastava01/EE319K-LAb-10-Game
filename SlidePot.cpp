// SlidePot.cpp
// Runs on TM4C123
// Provide functions that initialize ADC0
// Last Modified: 1/17/2021 
// Student names: Vibhor Srivastava and Tian-Da Huang
// Last Modified: 4/14/2021 

#include <stdint.h>
#include "SlidePot.h"
#include "../inc/tm4c123gh6pm.h"
// feel free to redesign this, as long as there is a class

// ADC initialization function 
// Input: sac sets hardware averaging
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(uint32_t sac){ 
	SYSCTL_RCGCGPIO_R |= 0x10;
	while((SYSCTL_PRGPIO_R & 0x10) == 0);
	GPIO_PORTE_DIR_R &= ~0x10;
	GPIO_PORTE_AFSEL_R |= 0x10;
	GPIO_PORTE_DEN_R &= ~0x10;
	GPIO_PORTE_AMSEL_R |= 0x10;
	SYSCTL_RCGCADC_R |= 0x01;
	int delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	ADC0_PC_R = 0x01;
	ADC0_SSPRI_R = 0x0123;
	ADC0_ACTSS_R &= ~0x0008;
	ADC0_EMUX_R &= ~0xF000;
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFFFFFF0) + 9;
	ADC0_SSCTL3_R = 0x0006;
	ADC0_IM_R &= ~0x0008;
	ADC0_ACTSS_R |= 0x0008;
	ADC0_SAC_R = sac;	// hardware averaging
}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	ADC0_PSSI_R = 0x0008;
	while((ADC0_RIS_R & 0x08) == 0);
	uint32_t data = ADC0_SSFIFO3_R & 0x0FFF;
	ADC0_ISC_R = 0x0008;
	return data;
}

// constructor, invoked on creation of class
// m and b are linear calibration coefficents 
SlidePot::SlidePot(uint32_t m, uint32_t b){
// initialize all private variables
// make slope equal to m and offset equal to b
	flag = 0;
	slope = m;
	offset = b;
}

void SlidePot::Save(void){
// 1) save ADC sample into private variable
// 2) calculate distance from ADC, save into private variable
// 3) set semaphore flag = 1
	data = ADC_In();
	distance = SlidePot::Convert(data);
	flag = 1;
}
//**********place your calibration data here*************
// distance PD2       ADC  fixed point
// 0.00cm   0.000V     0        0
// 0.50cm   0.825V  1024      500
// 1.00cm   1.650V  2048     1000
// 1.50cm   2.475V  3072     1500  
// 2.00cm   3.300V  4095     2000 
uint32_t SlidePot::Convert(uint32_t n){
  // use calibration data to convert ADC sample to distance
  return ((slope * n) / 4096) + offset;
}

void SlidePot::Sync(void){
// 1) wait for semaphore flag to be nonzero
// 2) set semaphore flag to 0
	while(!flag);
	flag = 0;
}

uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)
  // return last calculated ADC sample
  return data;
}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 200), 0.01cm
  // return last calculated distance in 0.001cm
  return distance;
}

