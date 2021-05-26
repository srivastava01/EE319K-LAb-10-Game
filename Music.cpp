// Music.c
// This program can use timer0A and timer1A ISR  
// playing your favorite song.
//
// For use with the TM4C123
// EE319K lab6 extra credit
// Program written by: Tian-Da Huang, Vibhor Srivastava
// 3/25/21 

// #include "Sound.h"
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Music.h"
#include "DAC.h"

extern "C" void TIMER0A_Handler(void);
extern "C" void TIMER1A_Handler(void);

const uint8_t sin_wave[32] = {
  8,9,11,12,13,14,14,15,15,15,14,
  14,13,12,11,9,8,7,5,4,3,2,
  2,1,1,1,2,2,3,4,5,7};

// Songs
const note_t TestScale[] = {
	{A0, Ei},
	{B0, Ei},
	{DF, Ei},
	{D, Ei},
	{E, Ei},
	{GF, Ei},
	{AF, Ei},
	{A, Ei},
	{0, 0}
};

const note_t Chirp[] = {
	{AF0, H}, {B0, Q}, {GF0, H}, {E0, Ei}, {GF0, Ei}, {AF0, H}, {B0, Q}, {GF0, D_H},
	{AF0, H}, {B0, Q}, {GF, H}, {E, Q}, {B0, H}, {A0, Ei}, {AF0, Ei}, {GF0, H},
	{E0, Ei}, {GF0, Ei}, {AF0, H}, {B0, Q}, {GF0, H}, {E0, Ei}, {GF0, Ei}, {AF0, H}, {B0, Q}, {GF0, D_H},
	{AF0, H}, {B0, Q}, {GF, H}, {E, Q}, {B, D_H}, {0, 0}
};

const note_t SongPlaceholder[] = {
	{G7, Ei}, {C7, Ei}, {E0, Ei}, {G0, Ei}, {C0, Ei}, {E, Ei}, {G, D_Q}, {E, D_Q},
	{AF7, Ei}, {C7, Ei}, {EF0, Ei}, {AF0, Ei}, {C0, Ei}, {EF, Ei}, {AF, D_Q}, {EF, D_Q},
	{BF7, Ei}, {D0, Ei}, {F0, Ei}, {BF0, Ei}, {D, Ei}, {F, Ei}, {BF, D_Q}, {0, Sp},
	{BF, Ei}, {0, Sp}, {BF, Ei}, {0, Sp}, {BF, Ei}, {C, D_Q}, {C, D_Q}, {0, 0}
};

const note_t TestJump[] = {
	{DF0, Z}, {D0, Z}, {D0, Z}, {E0, Z}, {GF0, Z},{AF0, Z},
	{BF0, Z}, {C0, Z}, {D, Z}, {D, Z}, {D, Z}, {0, 0}
};

const note_t Death[] = {
	{F7, Ei}, {C6, Q}, {0, 0}
};

// Envelope, lasts for 40*25ms = 1000ms
//const uint8_t envelope[40] = {
//	20, 40, 60, 80, 100, 100, 100, 100, 100, 100,
//	100, 100, 100, 100, 100, 100, 88, 78, 69,
//	61, 54, 47, 42, 37, 32, 29, 25, 22, 20,
//	17, 15, 14, 12, 11, 9, 8, 7, 6, 5, 0
//};

// global variables
uint32_t wave_ind;
uint32_t song_ind;
note_t* current_song;

// ***************** Timer_Init ****************
// Configure Timer0A, Timer1A interrupts (but do not activate)
// Inputs: none
// Outputs: none
void Timer_Init(void){
	SYSCTL_RCGCTIMER_R |= 0x00000007;
	__asm__{
		NOP
		NOP
		NOP
		NOP
	}
	TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
	TIMER1_CTL_R = 0x00000000;
  TIMER1_CFG_R = 0x00000000;
  TIMER1_TAMR_R = 0x00000002;
	// interrupt priority order: timer0 (dac) << timer1 (metronome)
	NVIC_PRI4_R = (NVIC_PRI4_R & 0x00FFFFFF) | 0x20000000;	// priority for TIMER0
	NVIC_PRI5_R = (NVIC_PRI5_R & 0xFFFF00FF) | 0x00006000;	// priority for TIMER1
	// enable interrupts for TIMER0A, TIMER1A
	NVIC_EN0_R |= 0x00280000;
}
void Timer_Stop(void){ 
	DAC_Out(0);
	TIMER0_IMR_R = 0;
	TIMER1_IMR_R = 0;
	TIMER0_CTL_R = 0;
	TIMER1_CTL_R = 0;
}

void Music_Init(void){
  DAC_Init();
	Timer_Init();
	song_ind = 0;
	wave_ind = 0;
}

// Play song, will run song until song end or Music_StopSong is called
void Music_PlaySong(note_t* song){
	current_song = song;
	TIMER0_TAILR_R = current_song[song_ind].pitch - 1;
	TIMER1_TAILR_R = current_song[song_ind].duration - 1;
	TIMER0_CTL_R = 1;
	TIMER1_CTL_R = 1;
	TIMER0_IMR_R = 1;
	TIMER1_IMR_R = 1;
}

// Stop song
void Music_StopSong(void){
  Timer_Stop();
	wave_ind = 0;
	song_ind = 0;
}

// Timer0A actually controls outputting to DAC
void TIMER0A_Handler(void){
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;
	wave_ind = (wave_ind + 1) % 32;
	//uint8_t final_sound = sin_wave[wave_ind];
	DAC_Out(sin_wave[wave_ind]);
}

// Timer1A acts as a metronome for the song(s)
void TIMER1A_Handler(void){
	TIMER1_ICR_R = TIMER_ICR_TATOCINT;
	song_ind++;
	// GPIO_PORTF_DATA_R ^= 0x00000008;
	if (current_song[song_ind].duration == 0) {
		Music_StopSong();
		return;
	}
	TIMER0_TAILR_R = current_song[song_ind].pitch - 1;
	TIMER1_TAILR_R = current_song[song_ind].duration - 1;
}
