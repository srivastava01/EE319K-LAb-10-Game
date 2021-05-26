// movement.cpp
// Acceleration-Based Movement Engine
// Defines the PhysicsObject Class and functions to control movement

#include "movement.h"
#include <stdint.h>
#include "SSD1306.h"
#include "images.h"
#include "Collisions.h"

Obstacles emptymap[1] = {Obstacles(0,0,0,0)};

PhysicsObj::PhysicsObj(const uint8_t *spriteptr, uint8_t xsize, uint8_t ysize) : 
	airtime(0), visible(false) {
		size.x = xsize;
		size.y = ysize;
		sprite = spriteptr;
		current_map = emptymap;	// set map to empty map, need to use setMap function
}

void PhysicsObj::updatePos(int8_t x, int8_t y) {
	// calculate points for collision box
//	Point<uint8_t> collision_box[8] = {	// 8 point collision box
//		pos, Point<uint8_t>(pos.x + size.x - 1, pos.y + size.y - 1), 
//		Point<uint8_t>((pos.x + size.x - 1)/2, pos.y), Point<uint8_t>((pos.x + size.x - 1)/2, pos.y + size.y - 1), 
//		Point<uint8_t>(pos.x, (pos.y + size.y - 1)/2), Point<uint8_t>(pos.x + size.x - 1, (pos.y + size.y - 1)/2), 
//		Point<uint8_t>(pos.x, pos.y + size.y - 1), Point<uint8_t>(pos.x + size.x - 1, pos.y)
//	};
	Point<uint8_t> collision_box[4] = {	// 4 corners collision box
		pos, Point<uint8_t>(pos.x + size.x - 1, pos.y + size.y - 1), 
		Point<uint8_t>(pos.x, pos.y + size.y - 1), Point<uint8_t>(pos.x + size.x - 1, pos.y)
	};
	
	Point<bool> collide(false, false); // collision state
	Obstacles *xstop = 0;
	Obstacles *ystop = 0;
	for (int i = 0; i < 4; i++) {	// x collision check
		xstop = xCollide(collision_box[i].x, collision_box[i].y, vel.x, current_map);
		if (xstop) {
			collide.x = true;
			break;
		}
	}
	for (int i = 0; i < 4; i++) {	// x collision check
		ystop = yCollide(collision_box[i].x, collision_box[i].y, vel.y, current_map);
		if (ystop) {
			collide.y = true;
			break;
		}
	}
//	xstop = xCollide(pos.x, pos.y + vel.y, vel.x, current_map);
//		if (xstop != 0) {
//			collide.x = true;
//		}
//	ystop = yCollide(pos.x + vel.x, pos.y, vel.y, current_map);
//		if (ystop != 0) {
//			collide.y = true;
//		}
	if (vel.x > 0) {
		if (collide.x) {	// Right collision
			pos.x = xstop->x1 - size.x;
		}
		else {
			pos.x += vel.x * VEL_MULT;
		}
	}
	else if (vel.x < 0) {
		if (collide.x) {	// Left collision
			pos.x = xstop->x1 + 1;
		}
		else {
			pos.x += vel.x * VEL_MULT;
		}
	}
	if (vel.y > 0) {
		if (collide.y) {	// Above collision
			vel.y = 0;
			pos.y = ystop->y1 - size.y;
		}
		else {
			pos.y += vel.y * VEL_MULT;
		}
	}
	else if (vel.y <= 0) {
		if (collide.y) {	// Under collision
			vel.y = 0;
			airtime = 0;
			pos.y = ystop->y1 + 1;
		}
		else {
			pos.y += vel.y * VEL_MULT;
		}
	}
	vel.x += x;
	vel.y += y;
}

void PhysicsObj::syncPos(void) {
	pos_old.x = pos.x;
	pos_old.y = pos.y;
}

void PhysicsObj::forcePos(uint8_t x, uint8_t y) {
	pos.x = x;
	pos.y = y;
}

void PhysicsObj::setVelX(int x) {
	vel.x = x;
}

void PhysicsObj::setVelY(int y) {
	vel.y = y;
}

#define MAXJUMP 4
void PhysicsObj::jump(void) {
	if (airtime == MAXJUMP) return;
	this->setVelY(5);
	airtime++;
}

void PhysicsObj::toggleVis(void) {
	visible = !visible;
}

void PhysicsObj::draw(void) {
	if (!visible) return;
	SSD1306_DrawBMP(pos_old.x, 63 - pos_old.y, sprite, 0, SSD1306_WHITE);
	// SSD1306_OutBuffer();
}

void PhysicsObj::erase(void) {
	if (!visible) return;
	for (uint8_t i = 0; i < size.x; i++) {
		for (uint8_t j = 0; j < size.y; j++) {
			SSD1306_DrawPixel(pos_old.x + i, 63 - pos_old.y - j, SSD1306_BLACK);
		}
	}
//	for (uint8_t i = 0; i < size.x; i++) {
//		SSD1306_DrawFastVLine(pos_old.x + i, 64 - pos_old.y - size.y, size.y, SSD1306_BLACK);
//	}
	// SSD1306_OutBuffer();
}

void PhysicsObj::setMap(Obstacles *map) {
	current_map = map;
}

bool PhysicsObj::nextMap(void) {
	if ((pos.x + size.x) >= 127) {
		this->toggleVis();
		this->setVelY(0);
		return true;
	}
	return false;
}

bool PhysicsObj::death(void) {
	if (pos.y <= 1) {
		this->toggleVis();
		this->setVelY(0);
		return true;
	}
	return false;
}

void PhysicsObj::oscillate(uint8_t thresh) {
	if (pos.x > thresh + 10) {
		this->setVelX(-1);
	}
	else if (pos.x < thresh - 10) {
		this->setVelX(1);
	}
}