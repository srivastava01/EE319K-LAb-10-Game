#include <stdint.h>
#include "SSD1306.h"
#include "Collisions.h"

void PrintVVec(Obstacles *vec){
	SSD1306_DrawFastVLine(vec->x1, 63 - vec->y2, vec->y2 - vec->y1 + 1, SSD1306_WHITE);
}

void PrintHVec(Obstacles *vec){
	SSD1306_DrawFastHLine(vec->x1, 63 - vec->y1, vec->x2 - vec->x1 + 1, SSD1306_WHITE);
}
#define PF1       (*((volatile uint32_t *)0x40025008))



void PrintMap(Obstacles *map){
	// PF1 ^= 0x02;
	SSD1306_ClearBuffer();
	Obstacles *map_ptr = map;
	while ((map_ptr->x1 != 0) || (map_ptr->x2 != 0) || (map_ptr->y1 != 0) || (map_ptr->y2 != 0)) {
		// PF1 ^= 0x02;
		if (map_ptr->x1 == map_ptr->x2) {
			PrintVVec(map_ptr);
		}
		else if (map_ptr->y1 == map_ptr->y2) {
			PrintHVec(map_ptr);
		}
		map_ptr++;
	}
	SSD1306_OutBuffer();
}

int abs(int x){
	return x < 0 ? -x : x;
}

Obstacles left(0, 0, 0, 63);
Obstacles right(127, 127, 0, 63);
Obstacles bottom(0, 127, 0, 0);
Obstacles top(0, 127, 63, 63);

Obstacles* xCollide(int xPos, int yPos, int vel, Obstacles *map){ // true if collision in x direction
	Obstacles *map_ptr = map;
	while ((map_ptr->x1 != 0) || (map_ptr->x2 != 0) || (map_ptr->y1 != 0) || (map_ptr->y2 != 0)){
		if (xPos + vel > 127) return &right;
		if (xPos + vel < 0) return &left;
		if (map_ptr->y1 == map_ptr->y2) {	// y1 == y2 means horizontal line; do nothing
		}
		else if (((vel <= 0) && (map_ptr->x1 - xPos >= vel) && (map_ptr->x1 - xPos < 0)) || ((vel > 0) && (map_ptr->x1 - xPos <= vel) && (map_ptr->x1 - xPos > 0))){
			if (yPos >= map_ptr->y1 && yPos <= map_ptr->y2) {
				return map_ptr;
			}
		}
		map_ptr++;
	}
	return 0;
}
		 
Obstacles* yCollide(int xPos, int yPos, int vel, Obstacles *map){ // true if collision in y direction
	Obstacles *map_ptr = map;
	while ((map_ptr->x1 != 0) || (map_ptr->x2 != 0) || (map_ptr->y1 != 0) || (map_ptr->y2 != 0)){
		if (yPos + vel > 63) return &top;
		if (yPos + vel < 0) return &bottom;
		if (map_ptr->x1 == map_ptr->x2) {	// x1 == x2 means vertical line; do nothing
		}
		else if (((vel <= 0) && (map_ptr->y1 - yPos >= vel) && (map_ptr->y1 - yPos < 0)) || ((vel > 0) && (map_ptr->y1 - yPos <= vel) && (map_ptr->y1 - yPos > 0))){
			if (xPos >= map_ptr->x1 && xPos <= map_ptr->x2) {
				return map_ptr;
			}
		}
		map_ptr++;
	}
	return 0;
}