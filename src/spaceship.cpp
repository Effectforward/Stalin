#include "raylib.h"
#include <spaceship.hpp>

// defines the constructor
Spaceship::Spaceship() {
  // loads the Spaceship.png into the image variable
  image = LoadTexture("assets/sprites/spaceship.png");
  // centres the Spaceship to the bottom
  /*subtracting image.width and image.height fixes minor positioning
  inconsistency that occurs becuase of the fact that the image will be drawn
  from top to left: When the image is drawn it is not drawn from the centre but
  instead from left most pixel, so the left most pixel becomes the centre*/
  position.x = (GetScreenWidth() - image.width) / 2.0;
  position.y = GetScreenHeight() - image.height;
  lastFireTime = 0.0;
}

Spaceship::~Spaceship() {
  // must unload the textures after use (kinda like heap memory)
  UnloadTexture(image);
}

void Spaceship::Draw() { DrawTextureV(image, position, WHITE); }

void Spaceship::moveLeft() {
  position.x -= speed;
  // avoids the spaceship escaping the game window
  if (position.x < 0) {
    position.x = 0;
  }
}

void Spaceship::moveRight() {
  position.x += speed;
  // avoids the spaceship escaping the game window
  if (position.x > GetScreenWidth() - image.width) {
    position.x = GetScreenWidth() - image.width;
  }
}

void Spaceship::Firelaser() {
  // creates laser beams at the centre
  // push_back basically adds an element to the end of a container, in this case
  // a vector

  if (GetTime() - lastFireTime >= 0.35) {

    lasers.push_back(
        Laser({position.x + (image.width / 2 - 2), position.y}, -15));
    lastFireTime = GetTime();
  }
}

Rectangle Spaceship::getRect() {
  // a rectangle that is positoned on the poosition of the sopaceship
  return {position.x, position.y, float(image.width), float(image.height)};
}