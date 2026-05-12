#pragma once
#include "entity.hpp"
#include "raylib.h"

class Alien : public Entity {
public:
  Alien(int type, Vector2 position);
  void Update(int direction);
  void Draw() override;
  int GetType();
  static void unloadImages();
  static Texture2D alienImages[3];
  int type;
  int hp;     // Health points: how many hits an alien can take
  int damage; // Damage level: how much impact it has on obstacles

  // rectangle for collison
  Rectangle getRect() override;

private:
};