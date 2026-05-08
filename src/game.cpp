
#include "aliens.hpp"
#include "laser.hpp"
#include "raylib.h"
#include <game.hpp>
#include <iostream>
Game::Game()
{
	aliens= CreateAliens();
}

Game::~Game()
{
	Alien :: unloadImages();
	alienDirection = 1;
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

	for(auto& alien: aliens){
		alien.Draw();

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

std::vector<Alien> Game::CreateAliens(){

			std::vector<Alien> aliens;
			for(int row = 0; row < 7 ; row++)
			{
		for (int column = 0; column < 10; column++)
		{
				int alienType;
				if(row == 0 ){
					alienType= 3;
				}
				else if (row == 1 || row == 2 ) {
					alienType = 2;
				
				}
				else {
					alienType=1 ;
				}

				float x = 75 +column * 55;
				float y = 110 + row * 55;
				aliens.push_back(Alien(alienType,{x, y} ));
		}
			}
			return aliens;
	}
	

void Game::moveAlien(){
	for(auto & alien : aliens ){
		alien.Update(alienDirection);
	}
}