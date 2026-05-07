/*Container for the all the game elements, makes the code easier to understand and debug*/
#pragma once
#include <spaceship.hpp>
#include <laser.hpp>
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
};