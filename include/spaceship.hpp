#pragma once

#include "laser.hpp"
#include "raylib.h"
#include <vector>
// Spaceship class
class Spaceship
{
  public:
	Spaceship();
	~Spaceship();
	void Draw();
	void moveLeft();
	void moveRight();
	void Firelaser();
	std::vector<Laser> lasers; // this will hold all the lasers

  private:
	int speed = 7;
	// for image rendering (it is a data structure)
	Texture2D image;
	// for positioning (it is also a data structure)
	Vector2 position;
	double lastFireTime;
};