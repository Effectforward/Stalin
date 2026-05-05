/*Container for the all the game elements, makes the code easier to understand and debug*/
#pragma once
#include <spaceship.hpp>
class Game
{
  public:
	Game();
	~Game();
	void Draw();
	void updatePosition();
	void handleInput();

  private:
	Spaceship spaceship;
};