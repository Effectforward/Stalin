#pragma once

#include "raylib.h"
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

  private:
	// for image rendering (it is a data structure)
	Texture2D image;
	// for poositioning (it is also a data structure)
	Vector2 position;
};