#include "raylib.h"
#include <spaceship.hpp>
#include <stalin.hpp>
// defines the constructor
Spaceship::Spaceship()
{
	// loads the Spaceship.png into the image variable
	image = LoadTexture("assets/sprites/spaceship.png");
	// centres the Spaceship to the bottom
	/*subtracting image.width and image.height fixes minor positioning inconsistency that occurs
	becuase of the fact that the image will be drawn from top to left: When the image is drawn it is
	not drawn from the centre but instead from left most pixel, so the left most pixel becomes the
	centre*/
	position.x = (GetScreenWidth() - image.width) / 2.0;
	position.y = GetScreenHeight() - image.height;
}

Spaceship::~Spaceship()
{
	// must unload the textures after use (kinda like heap memory)
	UnloadTexture(image);
}

void Spaceship::Draw()
{
	DrawTextureV(image, position, WHITE);
}
