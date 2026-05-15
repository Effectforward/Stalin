/*Container for the all the game elements, makes the code easier to understand
 * and debug*/
#pragma once
#include <aliens.hpp>
#include <laser.hpp>
#include <obstacles.hpp>
#include <spaceship.hpp>
#include <vector>

enum GameState { MENU, DIFFICULTY_SELECT, PLAYING, PAUSED, GAME_OVER };
enum Difficulty { NOVICE, EASY, MEDIUM, HARD, ULTRAVIOLENCE, STALIN, NIGHTMARE };
void drawPauseMenu();

class Game {
public:
  Game();
  ~Game();
  void Draw();
  void update();
  void handleInput();
  void deleteInactiveLasers();
  void checkForCollisions();
  void Reset();
  void nextLevel();
  bool run = true; // game running state
  GameState state;
  Difficulty difficulty;
  void drawPauseMenu();
  void drawScrollingBackground();

private:
  Spaceship spaceship;
  // parameterized constructor

  void moveAlien();
  void moveAliensDown(int distance);
  std::vector<Alien> CreateAliens();
  void alienShootLaser();
  std::vector<Alien> aliens;
  int alienDirection;
  std::vector<Laser> alienLaser;
  std::vector<Obstacle> createObstacles();
  // parameterized constructor
  Laser laser = Laser({100, 100}, -7);
  std::vector<Obstacle> obstacles;

  // --- NEW SPACE INVADERS MOVEMENT/SHOOTING VARIABLES ---
  // timeLastAlienMoved tracks the exact time (in seconds) the alien block last
  // took a step
  double timeLastAlienMoved;
  // alienMoveInterval defines how many seconds to wait before taking the next
  // step
  double alienMoveInterval;

  // timeLastAlienShot tracks the exact time (in seconds) the aliens last fired
  // a laser
  double timeLastAlienShot;
  // alienShootInterval defines the randomized wait time (in seconds) before the
  // next shot is fired
  double alienShootInterval;
  int lives; // lives of spaceship
  int score;
  int highScore;
  int level;
  Sound shootSound;
  Sound explosionSound;
  Sound alienDeathSound;
  Sound moveSounds[4];
  int currentMoveSound;
  Sound ufoLowSound;
  Sound ufoHighSound;
  
  // UFO state
  bool ufoActive;
  Vector2 ufoPosition;
  int ufoDirection;
  double nextUfoSpawnTime;
  Texture2D ufoTexture;

  void spawnUfo();
  void updateUfo();
  void drawUfo();
  void drawMenu();
  void handleMenuInput();
  void checkLevelCompletion();
  void gameOver();


   // --- MENU ---
    void drawMainMenu();
    void handleMainMenuInput();
    void drawDifficultyMenu();
    void handleDifficultyInput();

      int selectedDifficulty;   // 0-4, highlighted option
    float menuStarTimer;
    struct MenuStar {
        float x, y, speed, size;
    };
    MenuStar menuStars[80];
    void initMenuStars();
    void updateMenuStars();
    void drawMenuStars();
    int pauseSelected;


    // --- HIT FLASH ---
bool shipHit;
float shipHitTimer;
float shipHitDuration;  // how long the flash lasts
float shipImmunityTimer;
    
};