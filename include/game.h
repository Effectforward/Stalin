#ifndef GAME_H
#define GAME_H

#include "raylib.h"

struct GameState {
    float circle_x;
    float circle_y;
    float circle_radius;
    float circle_speed;
    int window_width;
    int window_height;
};

void initialize_game(GameState& state);
void update_game(GameState& state);
void draw_game(const GameState& state);

#endif
