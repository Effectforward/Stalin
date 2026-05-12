#pragma once

#include "laser.hpp"
#include "raylib.h"
#include <vector>
// Spaceship class
class Spaceship {
public:
  Spaceship();
  ~Spaceship();
  void Draw();
  void moveLeft();
  void moveRight();
  void Firelaser();
  // we need a rectangle to check collison uisng built in raylib function
  Rectangle getRect();
  void Reset();
  std::vector<Laser> lasers; // this will hold all the lasers

  // Power-up methods
  void setLaserSpeed(int s) { laserSpeed = s; }
  void setLaserDamage(int d) { laserDamage = d; }

private:
  int speed = 6;
  int laserSpeed = -15;  // Default speed
  int laserDamage = 1;   // Default damage
  // for image rendering (it is a data structure)
  Texture2D image;
  // for positioning (it is also a data structure)
  Vector2 position;
  double lastFireTime;
};