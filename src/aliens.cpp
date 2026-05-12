#include "aliens.hpp"
#include "raylib.h"

Texture2D Alien::alienImages[3] = {};
Alien::Alien(int type, Vector2 position) {
  this->type = type;
  this->position = position;
  if (alienImages[type - 1].id == 0) {
    // this needs a fix, the images should be resized before adding them to the
    // code, todo: resize aliens using magick

    // OLD CODE:
    // switch(type){
    //     case 1:
    //     alienImages[0] = LoadTexture("assets/sprites/e1.png");
    //     break;
    //      case 2:
    //     alienImages[1] = LoadTexture("assets/sprites/e2.png");
    //     break;
    //      case 3:
    //     alienImages[2] = LoadTexture("assets/sprites/e3i.png");
    //     break;
    //    default:
    //     alienImages[0]= LoadTexture("assets/sprites/e1i.png");
    //     break;
    // }
    // NEW CODE: Load image, resize it by half, and then create texture
    Image img;
    switch (type) {
    case 1:
      img = LoadImage("assets/sprites/e1.png");
      ImageResize(&img, img.width / 2, img.height / 2);
      alienImages[0] = LoadTextureFromImage(img);
      UnloadImage(img);
      break;
    case 2:
      img = LoadImage("assets/sprites/e2.png");
      ImageResize(&img, img.width / 2, img.height / 2);
      alienImages[1] = LoadTextureFromImage(img);
      UnloadImage(img);
      break;
    case 3:
      img = LoadImage("assets/sprites/e3i.png");
      ImageResize(&img, img.width / 2, img.height / 2);
      alienImages[2] = LoadTextureFromImage(img);
      UnloadImage(img);
      break;
    default:
      img = LoadImage("assets/sprites/e1i.png");
      ImageResize(&img, img.width / 2, img.height / 2);
      alienImages[0] = LoadTextureFromImage(img);
      UnloadImage(img);
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
  Color alienColor = WHITE;
  if (type == 1)
    alienColor = {88, 88, 175, 255}; // Purple/Blue
  else
    alienColor = YELLOW;

  DrawTextureV(alienImages[type - 1], position, alienColor);
}

int Alien::GetType() { return type; }

void Alien::unloadImages() {
  for (int i = 0; i < 4; i++) {

    UnloadTexture(alienImages[i]);
  }
}

void Alien::Update(int direction) { position.x += direction; }

Rectangle Alien::getRect() {

  return {position.x, position.y, float(alienImages[type - 1].width),
          float(alienImages[type - 1].height)};
}