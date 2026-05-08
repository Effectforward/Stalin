/*Container for the all the game elements, makes the code easier to understand and debug*/
#pragma once
#include <laser.hpp>
#include <obstacles.hpp>
#include <spaceship.hpp>
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
	std::vector<Obstacle> createObstacles();
	// parameterized constructor
	Laser laser = Laser({100, 100}, -7);
	std::vector<Obstacle> obstacles;
};