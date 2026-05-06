
#include "raylib.h"
#include <game.hpp>
Game::Game()
{
}

Game::~Game()
{
}

void Game::Draw()
{
	spaceship.Draw();
}

void Game::handleInput()
{
	if (IsKeyDown(KEY_LEFT)) {
		spaceship.moveLeft();
	} else if (IsKeyDown(KEY_RIGHT)) {
		spaceship.moveRight();
	}
}