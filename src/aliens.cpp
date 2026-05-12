#include "aliens.hpp"
#include "raylib.h"

Texture2D Alien::alienImages[3] = {};
Alien::Alien(int type, Vector2 position) {
  this->type = type;
  this->position = position;
  if (alienImages[type - 1].id == 0) {
    // this needs a fix, the images should be resized before adding them to the
    // code, todo: resize aliens using magick

    switch (type) {
    case 1:
      alienImages[0] = LoadTexture("assets/sprites/e1.png");
      break;
    case 2:
      alienImages[1] = LoadTexture("assets/sprites/e2.png");
      break;
    case 3:
      alienImages[2] = LoadTexture("assets/sprites/e3.png");
      break;
    default:
      alienImages[0] = LoadTexture("assets/sprites/e1.png");
      break;
    }
  }

  // Assign Health and Damage stats based on the alien type
  // Type 3 (Squid) is a tank, Type 1 is a grunt.
  switch (type) {
  case 3:
    hp = 3;
    damage = 30;
    break;
  case 2:
    hp = 2;
    damage = 20;
    break;
  case 1:
    hp = 1;
    damage = 10;
    break;
  default:
    hp = 1;
    damage = 10;
    break;
  }
}

void Alien::Draw() {
  int index = type - 1;
  if (index < 0) index = 0;
  if (index > 2) index = 2;
  
  DrawTextureV(alienImages[index], position, WHITE);
}

int Alien::GetType() { return type; }

void Alien::unloadImages() {
  for (int i = 0; i < 3; i++) {
    if (alienImages[i].id != 0) {
      UnloadTexture(alienImages[i]);
      alienImages[i].id = 0;
    }
  }
}

void Alien::Update(int direction) { position.x += direction; }

Rectangle Alien::getRect() {

  return {position.x, position.y, float(alienImages[type - 1].width),
          float(alienImages[type - 1].height)};
}