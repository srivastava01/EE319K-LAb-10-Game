// SlidePot.h
// Runs on TM4C123
// Provide functions that initialize ADC0 and use a slide pot to measure distance
// Created: 1/17/2021 
// Student names: Vibhor Srivastava and Tian-Da Huang
// Last Modified: 4/14/2021

#ifndef SLIDEPOT_H
#define SLIDEPOT_H
#include <stdint.h>
#define SAC_NONE 0
#define SAC_2 1
#define SAC_4 2
#define SAC_8 3
#define SAC_16 4
#define SAC_32 5
#define SAC_64 6
// feel free to redesign this, as long as there is a class
class SlidePot{ private:
    uint32_t data;     // raw ADC value
    int32_t flag;      // 0 if data is not valid, 1 if valid
    uint32_t distance; // distance in 0.001cm
// distance = (slope*data+offset)/4096
    uint32_t slope;    // calibration coeffients
    uint32_t offset;
public:
    SlidePot(uint32_t m, uint32_t b); // initialize slide pot
    void Save(void);        // save ADC, set semaphore
    void Sync(void);              // wait for semaphore
    uint32_t Convert(uint32_t n); // convert ADC to raw sample
    uint32_t ADCsample(void);     // return last ADC sample value (0 to 4095)
    uint32_t Distance(void);      // return last distance value (0 to 200), 0.01cm
};

// ADC initialization function, channel 5, PD2
// Input: none
// Output: none
void ADC_Init(uint32_t sac);

//------------ADC_In------------
// Busy-wait Analog to digital conversion, channel 5, PD2
// Input: sac sets hardware averaging
// Output: 12-bit result of ADC conversion
uint32_t ADC_In(void);

#endif

