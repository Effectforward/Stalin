
#include "laser.hpp"
#include "raylib.h"
#include <game.hpp>
#include <iostream>
Game::Game()
{
}

Game::~Game()
{
}

void Game::updatePosition()
{

	for (auto &laser : spaceship.lasers) {

		laser.Draw();
		laser.update();
		deleteInactiveLasers();
		std::cout << "Vector Size:" << spaceship.lasers.size() << std::endl;
	}
}

void Game::Draw()
{
	spaceship.Draw();
	for (auto &laser : spaceship.lasers) {

		laser.Draw();
	}
}

void Game::handleInput()
{
	if (IsKeyDown(KEY_LEFT)) {
		spaceship.moveLeft();
	} else if (IsKeyDown(KEY_RIGHT)) {
		spaceship.moveRight();
	}

	else if (IsKeyDown(KEY_SPACE)) {
		spaceship.Firelaser();
	}
}

void Game::deleteInactiveLasers()
{
	for (auto it = spaceship.lasers.begin(); it != spaceship.lasers.end();)
		if (!it->active) {
			it = spaceship.lasers.erase(it);
		} else
			++it;
}