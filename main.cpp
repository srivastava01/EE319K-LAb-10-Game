// main.cpp
// Runs on TM4C123
// This is a starter project for the EE319K Lab 10 in C++

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 47k  resistor DAC bit 0 on PB0 (least significant bit)
// 24k  resistor DAC bit 1 on PB1
// 12k  resistor DAC bit 2 on PB2
// 6k   resistor DAC bit 3 on PB3 
// 3k   resistor DAC bit 4 on PB4 
// 1.5k resistor DAC bit 5 on PB5 (most significant bit)

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "TExaS.h"
#include "SSD1306.h"
#include "SlidePot.h"
#include "Images.h"
#include "movement.h"
#include "print.h"
#include "Music.h"
#include "Collisions.h"
#include "Timer2.h"

//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PA54                  (*((volatile uint32_t *)0x400040C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF0       (*((volatile uint32_t *)0x40025004))
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PF4       (*((volatile uint32_t *)0x40025040))
#define PA5       (*((volatile uint32_t *)0x40004080)) 
#define PA4       (*((volatile uint32_t *)0x40004040)) 
// TExaSdisplay logic analyzer shows 7 bits 0,PA5,PA4,PF3,PF2,PF1,0 
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PA54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x21;      // activate port A,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  // GPIO_PORTF_DEN_R |=  0x1F;   // enable digital I/O on PF3,2,1
  GPIO_PORTA_DIR_R |=  0x30;   // output on PA4 PA5
  GPIO_PORTA_DEN_R |=  0x30;   // enable on PA4 PA5  
	// GPIO_PORTF_DIR_R &= ~0x11;	 // input on PF4, 0
	// GPIO_PORTF_PUR_R |= 0x11;
		// ???????
	GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
	GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
	GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
	GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
	GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
	GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
	GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
}
void SysTick_Init(unsigned long period){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_CURRENT_R = 0;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000;
	NVIC_ST_RELOAD_R = period - 1;
	NVIC_ST_CTRL_R = 0x00000007;
}
//********************************************************************************

// objects
SlidePot stick(9, 0);
PhysicsObj test(placeholder, 6, 8);
PhysicsObj treeprop(tree, 16, 24);
PhysicsObj cloudprop1(Cloud, 18, 10);
PhysicsObj cloudprop2(Cloud, 18, 10);

// timer
char numArray[3];
int timing = 30;
bool change = false;

// Map Collision Vectors
Obstacles map1[12] = {Obstacles(0,10,13,13),Obstacles(10,10,13,26),Obstacles(10,30,26,26),Obstacles(30,30,0,26),Obstacles(50,50,0,43),Obstacles(50,63,43,43),
Obstacles(63,63,30,43),Obstacles(63,90,30,30),Obstacles(90,90,0,30),Obstacles(110,110,0,48),Obstacles(110,127,48,48),Obstacles(0,0,0,0)};

Obstacles map2[13] = {Obstacles(0,10,13,13),Obstacles(10,10,0,13),Obstacles(30,30,0,23),Obstacles(30,43,23,23),
	Obstacles(43,43,23,45),Obstacles(43,73,45,45),Obstacles(73,73,0,45),
	Obstacles(90,90,0,30),Obstacles(105,105,0,30),
Obstacles(90,105,30,30),Obstacles(117,117,0,48),Obstacles(117,127,48,48),Obstacles(0,0,0,0)};

Obstacles map3[13] = {Obstacles(0,10,13,13),Obstacles(10,10,13,30),Obstacles(10,30,30,30),Obstacles(30,30,0,30),Obstacles(45,65,40,40),
	Obstacles(45,45,0,40),Obstacles(65,65,0,40),Obstacles(80,80,0,50),Obstacles(100,100,0,50),
Obstacles(80,100,50,50),Obstacles(115,115,0,40),Obstacles(115,127,40,40),Obstacles(0,0,0,0)};

Obstacles map4[22] = {Obstacles(0,10,13,13),Obstacles(10,10,13,25),Obstacles(10,22,25,25),Obstacles(22,22,25,37),Obstacles(22,34,37,37),
Obstacles(34,34,37,50),Obstacles(34,90,50,50),Obstacles(120,120,37,63),Obstacles(110,120,37,37),Obstacles(90,100,27,27),
Obstacles(90,90,0,27),Obstacles(100,100,0,27),Obstacles(60,70,17,17),Obstacles(60,60,0,17),Obstacles(70,70,0,17),
Obstacles(34,44,5,5),Obstacles(44,44,0,5),Obstacles(34,34,0,5),Obstacles(115,127,6,6),Obstacles(115,115,0,6),
Obstacles(127,127,0,6),Obstacles(0,0,0,0)};

char spanIntro[] = "Para espa\xA9""ol: PF0\n";
char spanDescribe[] = "\xB2Tienes que ir a clase o fallar\xA5s! Todo ha salido mal ...los Aggies han puesto obst\xA5""culos. Presione PF4 para saltar y mueva el slidepot para avanzar. PF4 para continuar->";
char spanWin1[]="LONGHORNS est\xA5n orgul";
char spanLose1[]="Longhorns est\xA5n decep";
extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);

void SysTick_Handler(void){ // every 50 ms
	// PF2 ^= 0x04;     // Heartbeat
	stick.Save();
	test.updatePos(0, -1);
	cloudprop1.oscillate(32);
	cloudprop1.updatePos(0, 0);
	cloudprop2.oscillate(96);
	cloudprop2.updatePos(0, 0);
}

int gameplayloop(Obstacles *map);

void output(void){
		change = true;
	  timing--;
}

void toArray(int number,char* numArray){
    for (int i = 2; i >= 0; --i, number /= 10){
        numArray[i] = (number % 10) + '0';
    }
}

void TouchRelease(int port) {
	if (port == 0) {
		while (!PF0);
		Delay1ms(5);
		while (PF0);
		Delay1ms(5);
		return;
	}
	if (port == 4) {
		while (!PF4);
		Delay1ms(5);
		while (PF4);
		Delay1ms(5);
		return;
	}
	return;
}

#define MS 80000
int main(void){
	Music_Init();
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  // PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  // TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
	ADC_Init(SAC_16);
	SysTick_Init(50 * MS);
	EnableInterrupts();
	int GameState = 0;
	bool lang = false;
	int maxTime = -1;
	// Gameplay Loop
	while(1) {
		switch (GameState) {
			case 0 : { // start screen, menu sequence
				Music_StopSong();
				timing = 30;
				SSD1306_ClearBuffer();
				SSD1306_OutClear();
				SSD1306_DrawBMP(2,62,gameScreen,0,SSD1306_WHITE);
				SSD1306_DrawString(15,55,(char*)"Press PF4 to Play!",SSD1306_WHITE);
				SSD1306_OutBuffer();
				TouchRelease(4);
				SSD1306_ClearBuffer();
				SSD1306_OutClear();
				SSD1306_OutString((char*)"For English: PF4\n");
				SSD1306_OutString((char*)spanIntro);
				int x = 10000000;
				while(x!=0){
					x--;
				}
				while(PF4&&PF0);
				if(!PF4){
					SSD1306_OutClear();
					lang = false;
					SSD1306_OutString((char*) "You need to get to class or else you'll fail! Everything has gone wrong...the Aggies messed up the road. Press PF4 to jump and use the slidepot to move. PF4 to continue->");
				}
				else if(!PF0){
					SSD1306_OutClear();
					lang = true;
					SSD1306_OutString((char*) spanDescribe);
					lang = true;
				}
				x = 10000000;
				while(x!=0){
					x--;
				}
				TouchRelease(4);
				SSD1306_ClearBuffer();
				SSD1306_OutClear();
				Timer2_Init(&output,1000*MS);
				GameState++;
				break;
			}
			case 1 : { // map 1
				int result = gameplayloop(map1);
				if (result < 0) {
					GameState = 0;
				}
				else if(result>=10){
					GameState = 6;
				}
				else {
					GameState += result;
				}
				
				break;
			}
			case 2 : { // map 2
				int result = gameplayloop(map2);
				if (result < 0) {
					GameState = 0;
				}
				else if(result>=10){
					GameState = 6;
				}
				else {
					GameState += result;
				}
				break;
			}
			case 3 : { // map 3
				int result = gameplayloop(map3);
				if (result < 0) {
					GameState = 0;
				}
				else if(result>=10){
					GameState = 6;
				}
				else {
					GameState += result;
				}
				break;
			}
			case 4 : { // map 4
				treeprop.toggleVis();
				cloudprop2.toggleVis();
				int result = gameplayloop(map4);
				treeprop.toggleVis();
				cloudprop2.toggleVis();
				if (result < 0) {
					GameState = 0;
				}
				else if(result>=10){
					GameState = 6;
				}
				else {
					GameState += result;
				}
				break;
			}
			case 5 : {	// end card
				SSD1306_ClearBuffer();
				SSD1306_OutClear();
				Music_PlaySong(SongPlaceholder);
				Timer2_Stop();
				if(!lang){
					if(timing>maxTime){
						toArray(timing,numArray);
						SSD1306_DrawString(0,50, (char*)"High Score: ",SSD1306_WHITE);
						SSD1306_DrawString(70,50, numArray,SSD1306_WHITE);
						maxTime = timing;
					}
					else{
						SSD1306_DrawString(0,50, (char*)"High Score: ",SSD1306_WHITE);
						char max[3];
						toArray(maxTime,max);
						SSD1306_DrawString(70,50,max,SSD1306_WHITE);
					}
					SSD1306_DrawString(0, 0, (char*)"Score: ", SSD1306_WHITE);
					SSD1306_DrawString(35, 0, numArray, SSD1306_WHITE);
					SSD1306_DrawString(0,10, (char*)"YOU BEAT THE AGGIES",SSD1306_WHITE);
					SSD1306_DrawString(0,20, (char*)"LONGHORN NATION IS",SSD1306_WHITE);
					SSD1306_DrawString(0,30, (char*)"PROUD! PF4 to play",SSD1306_WHITE);
					SSD1306_DrawString(0,40, (char*)"again,beat high score!",SSD1306_WHITE);
					SSD1306_OutBuffer();
					
				}
				else{
				if(timing>maxTime){
						SSD1306_DrawString(0,50, (char*)"Mejor Puntaje: ",SSD1306_WHITE);
						SSD1306_DrawString(90,50, numArray,SSD1306_WHITE);
						maxTime = timing;
					}
					else{
						SSD1306_DrawString(0,50, (char*)"Mejor Puntaje: ",SSD1306_WHITE);
						char max[3];
						toArray(maxTime,max);
						SSD1306_DrawString(90,50,max,SSD1306_WHITE);
					}
					SSD1306_DrawString(0, 0, (char*)"Puntaje: ", SSD1306_WHITE);
					SSD1306_DrawString(55, 0, numArray, SSD1306_WHITE);
					SSD1306_DrawString(0,10, (char*)"VENCES A LOS AGGIES",SSD1306_WHITE);
					SSD1306_DrawString(0,20, spanWin1,SSD1306_WHITE);
					SSD1306_DrawString(0,30, (char*)"-losas PF4 para jugar.",SSD1306_WHITE);
					SSD1306_DrawString(0,40, (char*)"Golpear mejor puntaje",SSD1306_WHITE);
					SSD1306_OutBuffer();
				}
				TouchRelease(4);
				GameState = 0;
				break;
			}
			case 6 : { // game over
				cloudprop1.toggleVis();
				cloudprop2.toggleVis();
				SSD1306_ClearBuffer();
				SSD1306_OutClear();
				Music_PlaySong(Chirp);
				Timer2_Stop();
				if(!lang){
					SSD1306_DrawString(0, 0, (char*)"YOU LOSE AND THE", SSD1306_WHITE);
					SSD1306_DrawString(0,10, (char*)"AGGIES WIN...You",SSD1306_WHITE);
					SSD1306_DrawString(0,20, (char*)"Disappointed Longhorn",SSD1306_WHITE);
					SSD1306_DrawString(0,30, (char*)"Nation, press PF4",SSD1306_WHITE);
					SSD1306_DrawString(0,40, (char*)"to redeem yourself",SSD1306_WHITE);
					SSD1306_DrawString(0,50, (char*)"and try again",SSD1306_WHITE);
					SSD1306_OutBuffer();
				}
				else{
					SSD1306_DrawString(0,0,  (char*)"TU PIERDES Y EL", SSD1306_WHITE);
					SSD1306_DrawString(0,10, (char*)"AGGIES GANAN...",SSD1306_WHITE);
					SSD1306_DrawString(0,20, spanLose1,SSD1306_WHITE);
					SSD1306_DrawString(0,30, (char*)"-ciondada, PF4 para",SSD1306_WHITE);
					SSD1306_DrawString(0,40, (char*)"intentar otra vez",SSD1306_WHITE);
					SSD1306_OutBuffer();
				}
				TouchRelease(4);
				GameState = 0;
				break;
			}
		}
	}
}

int gameplayloop(Obstacles *map) {	// return -1 to go to start, 0 to go restart level, 1 to advance
	cloudprop1.forcePos(10, 51);
	cloudprop2.forcePos(90, 51);
	test.forcePos(2, 15);
	test.setMap(map);
	PrintMap(map);
	test.toggleVis();
	cloudprop1.toggleVis();
	cloudprop2.toggleVis();
	while(!test.nextMap()) {
		if(timing == 0) {
			test.toggleVis();
			test.setVelY(0);
			return 10;
		}
		if(change){	// score
			toArray(timing,numArray);
			SSD1306_DrawString(0,0,numArray,SSD1306_WHITE);
		}
		if (test.death()) {
			Music_PlaySong(Death);
			cloudprop1.toggleVis();
			cloudprop2.toggleVis();
			return 0;
		}
		test.setVelX(stick.Distance() - 4);
		if (!PF4) {
			test.jump();
			Music_PlaySong(TestJump);
		}
		test.erase();
		test.syncPos();
		test.draw();
		cloudprop1.erase();
		cloudprop1.syncPos();
		cloudprop1.draw();
		cloudprop2.erase();
		cloudprop2.syncPos();
		cloudprop2.draw();
		treeprop.draw();
		SSD1306_OutBuffer();
	}
	cloudprop1.toggleVis();
	cloudprop2.toggleVis();
	return 1;
}