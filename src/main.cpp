#include "game.h"
#include <stdio.h>

int main()
{
	int x = 5;
	if (x > 0) {
		printf("test");
	}
	GameState game;
	initialize_game(game);

	InitWindow(game.window_width, game.window_height, "Game - ESC to exit");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_ESCAPE)) {
			break;
		}

		update_game(game);

		BeginDrawing();
		ClearBackground(BLACK);
		draw_game(game);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

void initialize_game(GameState &state)
{
	state.window_width = 800;
	state.window_height = 600;
	state.circle_x = state.window_width / 2.0f;
	state.circle_y = state.window_height / 2.0f;
	state.circle_radius = 30.0f;
	state.circle_speed = 200.0f;
}

void update_game(GameState &state)
{
	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
		state.circle_y -= state.circle_speed * GetFrameTime();
	}
	if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
		state.circle_y += state.circle_speed * GetFrameTime();
	}
	if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
		state.circle_x -= state.circle_speed * GetFrameTime();
	}
	if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
		state.circle_x += state.circle_speed * GetFrameTime();
	}

	if (state.circle_x - state.circle_radius < 0) {
		state.circle_x = state.circle_radius;
	}
	if (state.circle_x + state.circle_radius > state.window_width) {
		state.circle_x = state.window_width - state.circle_radius;
	}
	if (state.circle_y - state.circle_radius < 0) {
		state.circle_y = state.circle_radius;
	}
	if (state.circle_y + state.circle_radius > state.window_height) {
		state.circle_y = state.window_height - state.circle_radius;
	}
}

void draw_game(const GameState &state)
{
	DrawCircle((int)state.circle_x, (int)state.circle_y, state.circle_radius, GREEN);

	DrawText("Use WASD or Arrow Keys to move", 10, 10, 20, WHITE);
	DrawText("Press ESC to exit", 10, 35, 20, WHITE);
	DrawText(TextFormat("FPS: %i", GetFPS()), 10, state.window_height - 30, 20, WHITE);
}
