#pragma once
#include <block.hpp>
#include <raylib.h>
#include <vector>
class Obstacle {
public:
  Obstacle(Vector2 position);
  void Draw();
  Vector2 position;
  std::vector<Block> blocks;
  // static make sures that we can access it witout having to create an objecti
  static std::vector<std::vector<int>> grid;

private:
};