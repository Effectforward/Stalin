#pragma once
#include <aliens.hpp>
#include <laser.hpp>
#include <obstacles.hpp>
#include <spaceship.hpp>
#include <vector>
#include <string>

enum GameState {
  MENU,
  DIFFICULTY_SELECT,
  CUSTOM_MODE,
  PLAYING,
  PAUSED,
  GAME_OVER
};

enum Difficulty {
  NOVICE,
  EASY,
  MEDIUM,
  HARD,
  ULTRAVIOLENCE,
  STALIN,
  NIGHTMARE,
  CUSTOM
};

// ---------- Custom Mode Settings ----------
struct CustomSettings {
  int   rows          = 5;      // 1-15
  int   cols          = 11;     // 1-20
  int   lives         = 3;      // 1-20
  float alienSpeed    = 0.6f;   // move interval in seconds (lower = faster)
  float fireRateScale = 1.0f;   // multiplier on shoot interval
  int   laserSpeed    = -20;    // negative = upward
  int   laserDamage   = 1;      // 1-10
  int   alienHP       = 1;      // 1-5
  int   stepSize      = 15;     // 5-40 px
  int   descentSpeed  = 15;     // 5-40 px
  float gridStartY    = 0.15f;  // 0.05-0.50 (fraction of screen height)
  float alienSpacing  = 55.0f;  // 30-80 px
  int   startingLevel = 1;      // 1-20
  float ufoFrequency  = 15.0f;  // 5-60 seconds
  float levelSpeedScale = 0.9f; // 0.5-1.0 multiplier per level
  int   shieldCount   = 4;      // 0-6
  float scoreMultiplier = 1.0f; // 1x-5x
};

// ---------- Named Preset ----------
struct CustomPreset {
  char           name[32];
  CustomSettings settings;
};

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
  bool shouldQuit = false;
  GameState state;
  Difficulty difficulty;

  // pause menu needs to be public for draw
  void drawPauseMenu();
  void drawScrollingBackground();

private:
  // ---- core objects ----
  Spaceship spaceship;
  std::vector<Alien>    aliens;
  std::vector<Laser>    alienLaser;
  std::vector<Obstacle> obstacles;
  Laser laser = Laser({100, 100}, -7); // keep to avoid linker issues

  int alienDirection;

  // ---- alien movement / shooting ----
  double timeLastAlienMoved;
  double alienMoveInterval;
  double timeLastAlienShot;
  double alienShootInterval;

  // ---- game state ----
  int lives;
  int score;
  int highScore;
  int level;

  // ---- audio ----
  Sound shootSound;
  Sound explosionSound;
  Sound alienDeathSound;
  Sound moveSounds[4];
  int   currentMoveSound;
  Sound ufoLowSound;
  Sound ufoHighSound;

  // ---- UFO ----
  bool     ufoActive;
  Vector2  ufoPosition;
  int      ufoDirection;
  double   nextUfoSpawnTime;
  Texture2D ufoTexture;
  void spawnUfo();
  void updateUfo();
  void drawUfo();

  // ---- core helpers ----
  void moveAlien();
  void moveAliensDown(int distance);
  std::vector<Alien>    CreateAliens();
  void                  alienShootLaser();
  std::vector<Obstacle> createObstacles();
  void checkLevelCompletion();
  void gameOver();

  // ---- legacy menu (kept so nothing breaks) ----
  void drawMenu();
  void handleMenuInput();

  // ---- main menu ----
  void drawMainMenu();
  void handleMainMenuInput();

  // ---- difficulty select ----
  void drawDifficultyMenu();
  void handleDifficultyInput();
  int  selectedDifficulty;

  // ---- custom mode ----
  CustomSettings customSettings;
  int  customSelected;   // which row is highlighted
  int  customPage;       // 0 or 1 (we split 17 params over 2 pages)
  void drawCustomMenu();
  void handleCustomMenuInput();
  void resetCustom();

  // ---- presets ----
  static constexpr int MAX_PRESETS = 10;
  CustomPreset presets[MAX_PRESETS];
  int          presetCount = 0;
  bool         namingPreset = false;   // are we typing a preset name?
  char         presetNameBuf[32] = {}; // input buffer
  int          presetNameLen = 0;
  void savePresets();
  void loadPresets();
  void drawPresetPanel();       // rendered on right side of custom menu
  void handlePresetInput();

  // ---- star background ----
  struct MenuStar {
    float x, y, speed, size, drift;
  };
  MenuStar menuStars[80];
  void initMenuStars();
  void updateMenuStars();
  void drawMenuStars();

  // ---- pause menu ----
  int pauseSelected;

  // ---- hit flash ----
  bool  shipHit;
  float shipHitTimer;
  float shipHitDuration;
  float shipImmunityTimer;
};