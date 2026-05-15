#include <game.hpp>
#include <raylib.h>

int main() {
  Color grey = {29, 29, 27, 255};
  const int windowHeight = 750;
  const int windowWidth = 700;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE );

  InitWindow(windowWidth, windowHeight, "Stalin");
  SetExitKey(KEY_NULL); 

  ToggleFullscreen();

  SetWindowMinSize(700, 750);
  InitAudioDevice(); // Initialize audio
  // limits the fps or the game will run at maximum speed
  SetTargetFPS(60);
  Game game;

  // game loop
  while (!WindowShouldClose() && !game.shouldQuit) {

    game.handleInput();
    // canvas is ready for drawing stuff
    BeginDrawing();
    // by defualt background is black, this clears it with our custom color
    ClearBackground(BLACK);
    game.Draw();
    game.update();

    // canvas is closed
    EndDrawing();
  }
  // window should be closed after an iniliatization
  CloseAudioDevice();
  CloseWindow();

  return 0;
}