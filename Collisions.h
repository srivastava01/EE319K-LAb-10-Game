#ifndef COLLISIONS_H
#define COLLISIONS_H	
#include <stdint.h>
#include "SSD1306.h"


class Obstacles{	// map is null terminated array of obstacles
	public:
		int x1,x2,y1,y2;
	
		Obstacles() {}
	
		Obstacles(int a, int b, int c, int d){
				 this->x1 = a;
				 this->x2 = b;
				 this->y1 = c;
				 this->y2 = d;
		}
};



void PrintMap(Obstacles *map);
Obstacles* xCollide(int xPos, int yPos, int vel, Obstacles *map); 
Obstacles* yCollide(int xPos, int yPos, int vel, Obstacles *map);
 
#endif