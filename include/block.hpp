#pragma once
#include "raylib.h"

class Block {
public:
  Block(Vector2 position);
  void Draw();
  // an invisible rectangle will be used to check collosion
  Rectangle getRect();

private:
  Vector2 position;
};