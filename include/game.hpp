/*Container for the all the game elements, makes the code easier to understand and debug*/
#pragma once
#include <spaceship.hpp>
#include <laser.hpp>
#include <aliens.hpp>
#include <vector>
class Game
{
  public:
	Game();
	~Game();
	void Draw();
	void updatePosition();
	void handleInput();
	void deleteInactiveLasers();

  private:
	Spaceship spaceship;
	//parameterized constructor
	Laser laser = Laser({100,100}, -7);
	void moveAlien();

	std::vector<Alien> CreateAliens();
	std::vector<Alien> aliens;
	int alienDirection;
};