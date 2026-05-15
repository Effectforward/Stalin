#pragma once
#include "raylib.h"

// Base class for all game entities to demonstrate Inheritance and Polymorphism
class Entity {
public:
    Vector2 position;
    Texture2D texture;
    
    virtual ~Entity() {}
    virtual void Draw() = 0; // Pure virtual function
    virtual Rectangle getRect() = 0;
};
