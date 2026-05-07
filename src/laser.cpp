#include "raylib.h"
#include <iostream>
#include <laser.hpp>
#include <ostream>
Laser::Laser(Vector2 position, int speed)
{
	this->position = position;
	this->speed = speed;
	active = true;
}

void Laser::Draw()
{
	if (active) {

		DrawRectangle(position.x, position.y, 4, 5, YELLOW);
	}
}

void Laser::update()
{
	position.y += speed;
	// makes the laser inactive when it is outside the window, so it won't be drawn infinitely
	if (active) {
		if (position.y > GetScreenWidth() || position.y < 0)
			active = false;
		std::cout << "Laser inactive" << std::endl;
	}
}