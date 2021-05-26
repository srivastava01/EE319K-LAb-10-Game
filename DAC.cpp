// DAC.c
// This software configures DAC output
// Lab 6 requires a minimum of 4 bits for the DAC, but you could have 5 or 6 bits
// Runs on TM4C123
// Program written by: Tian-Da Huang, Vibhor Srivastava
// Date Created: 3/6/17 
// Last Modified: 3/25/21 
// Lab number: 6
// Hardware connections
// 4-bit DAC, 4 button keys + 1 button for song playback, 2 status LEDs

#include <stdint.h>
#include "DAC.h"
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	SYSCTL_RCGC2_R |= 0x00000002;
	__asm__{
		NOP
		NOP
	}
	GPIO_PORTB_DIR_R |= 0x0000000F;
	GPIO_PORTB_DEN_R |= 0x0000000F;
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint8_t data){
	GPIO_PORTB_DATA_R = (data & 0x0000000F);
}
