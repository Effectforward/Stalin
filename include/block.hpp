#pragma once
#include "raylib.h"

class Block{
    public:
    Block(Vector2 position);
    void Draw();
    private:
    Vector2 position;
};