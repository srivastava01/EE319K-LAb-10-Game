// movement.h
// Acceleration-Based Movement Engine
// Defines the PhysicsObj Class and functions to control movement

#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <stdint.h>
#include "Collisions.h"
#define VEL_MULT 1	// multiplier for velocity 

class PhysicsObj{
private:
	template <class T>
	class Point{
	public: 
		T x;
		T y;
		Point() : x(0), y(0) {};
		Point(T xin, T yin) {
			this->x = xin;
			this->y = yin;
		}
	};
	Point<uint8_t> pos;
	Point<uint8_t> pos_old;
	Point<int> vel;
	uint8_t airtime;
	const uint8_t *sprite;
	Point<uint8_t> size;	// height and width of sprite in pixels
	bool visible;
	Obstacles *current_map;
public:
	PhysicsObj(const uint8_t *spriteptr, uint8_t xsize, uint8_t ysize);
	void updatePos(int8_t x, int8_t y);	// input acceleration, should be called periodically
	void syncPos(void);	// update pos_old = pos, called asynchronously
	void forcePos(uint8_t x, uint8_t y);	// does not check for collision or redraw sprite
	void setVelX(int x);		// override velocity, does not redraw sprite
	void setVelY(int y);		// override velocity, does not redraw sprite
	void jump(void);
	void toggleVis(void);					// set sprite visibility flag
	void draw(void);
	void erase(void);
	void setMap(Obstacles *map);
	bool nextMap(void);
	bool death(void);
	void oscillate(uint8_t thresh);
};

#endif