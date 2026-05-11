#pragma once
#include <raylib.h>
class Laser {
public:
  Laser(Vector2 position, int speed, int damage = 10);
  void update();
  void Draw();
  bool active; // active means inside the game window
  Rectangle getRect();
  int damage; // How much damage this laser deals

private:
  Vector2 position;
  int speed;
};