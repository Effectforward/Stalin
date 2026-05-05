#include <raylib.h>
#include <game.hpp>
#include <stalin.hpp>

int main()
{
	// defines custom color for the background
	Color grey = {29, 29, 27, 255};
	// initializes the window
	InitWindow(windowWidth, windowHeight, "Stalin");
	// limits the fps or the game will run at maximum speed
	SetTargetFPS(60);
	Game game;
	// game loop
	while (WindowShouldClose() == false) {
		// canvas is ready for drawing stuff
		BeginDrawing();
		// by defualt background is black, this clears it with our custom color
		ClearBackground(grey);
		game.Draw();
		// canvas is closed
		EndDrawing();
	}
	// window should be closed after an iniliatization
	CloseWindow();

	return 0;
}