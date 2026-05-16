#include "aliens.hpp"
#include "laser.hpp"
#include "obstacles.hpp"
#include "raylib.h"
#include <cstdint>
#include <cstring>

#include <game.hpp>

#include <vector>
Game::Game() {
  aliens = CreateAliens();
  alienDirection = 1;
  obstacles = createObstacles();

  // --- NEW SPACE INVADERS SETUP ---
  // Initialize movement timer. Starts off moving every half second.
  timeLastAlienMoved = GetTime();
  alienMoveInterval = 0.5;

  // Initialize shooting timer.
  timeLastAlienShot = GetTime();
  // Randomize the first shot between 1.0 and 3.0 seconds
  alienShootInterval = GetRandomValue(10, 30) / 10.0;
  lives = 3;
  score = 0;
  highScore = 0;
  level = 1;
  state = MENU;
  difficulty = MEDIUM;

  // Load Audio
  shootSound = LoadSound("assets/audio/shoot.wav");
  explosionSound = LoadSound("assets/audio/explosion.wav");
  alienDeathSound = LoadSound("assets/audio/invaderkilled.wav");
  moveSounds[0] = LoadSound("assets/audio/fastinvader1.wav");
  moveSounds[1] = LoadSound("assets/audio/fastinvader2.wav");
  moveSounds[2] = LoadSound("assets/audio/fastinvader3.wav");
  moveSounds[3] = LoadSound("assets/audio/fastinvader4.wav");
  ufoLowSound = LoadSound("assets/audio/ufo_lowpitch.wav");
  ufoHighSound = LoadSound("assets/audio/ufo_highpitch.wav");
  currentMoveSound = 0;

  ufoActive = false;
  nextUfoSpawnTime = GetTime() + GetRandomValue(10, 20);
  ufoTexture = LoadTexture("assets/sprites/ufo.png");
  if (ufoTexture.id == 0) {
    ufoTexture = LoadTexture("assets/sprites/e1.png"); // Fallback if no ufo.png
  }

  selectedDifficulty = 2;
  initMenuStars();
  pauseSelected = 0;
  shipHit = false;
  shipHitTimer = 0.0f;
  shipHitDuration = 0.6f;
  shipImmunityTimer = 0.0f;

  customSelected = 0;
  customPage = 0;
  namingPreset = false;
  presetCount = 0;
  memset(presetNameBuf, 0, sizeof(presetNameBuf));
  presetNameLen = 0;
  loadPresets();
}

Game::~Game() {
  UnloadSound(shootSound);
  UnloadSound(explosionSound);
  UnloadSound(alienDeathSound);
  for (int i = 0; i < 4; i++) {
    UnloadSound(moveSounds[i]);
  }
  UnloadSound(ufoLowSound);
  UnloadSound(ufoHighSound);
  UnloadTexture(ufoTexture);
}

void Game::update() {
  if (state != PLAYING) // PAUSED, MENU, etc. all skip update
    return;
  // Hit flash timer
  if (shipHit) {
    shipHitTimer -= GetFrameTime();
    if (shipHitTimer <= 0.0f) {
      shipHit = false;
      shipHitTimer = 0.0f;
    }
  }

  if (shipImmunityTimer > 0.0f) {
    shipImmunityTimer -= GetFrameTime();
    if (shipImmunityTimer < 0.0f)
      shipImmunityTimer = 0.0f;
  }

  for (auto &laser : spaceship.lasers) {
    laser.update();
  }

  for (auto &laser : alienLaser) {
    laser.update();
  }

  checkForCollisions();
  deleteInactiveLasers();

  // POWER-UP LOGIC: Check if aliens are low
  float maxY = 0;
  for (const auto &alien : aliens) {
    if (alien.position.y > maxY)
      maxY = alien.position.y;
  }

  // If aliens cross 70% of the screen, boost the player's lasers
  if (difficulty != CUSTOM) {
    if (maxY > GetScreenHeight() * 0.7f) {
      spaceship.setLaserSpeed(-25); // Faster lasers
      spaceship.setLaserDamage(3);  // Triple damage
    }

    // If fewer than 10 aliens remain, go to hyper-speed
    if (aliens.size() < 25 && !aliens.empty()) {
      spaceship.setLaserSpeed(-40); // Hyper lasers
      spaceship.setLaserDamage(10); // Ultimate damage
    }
  }

  moveAlien();
  alienShootLaser();
  updateUfo();
  checkLevelCompletion();
}
void Game::Draw() {
  if (state == MENU) {
    drawMainMenu();
    return;
  }
  if (state == DIFFICULTY_SELECT) {
    drawDifficultyMenu();
    return;
  }

  if (state == CUSTOM_MODE) {
    drawCustomMenu();
    return;
  }

  // Draw game world (PLAYING, PAUSED, and GAME_OVER all show the game behind)
  drawScrollingBackground();
  // Flash ship red/transparent when hit
  if (shipImmunityTimer > 0.0f) {
    // Fast red strobe during hit flash window
    if (shipHit) {
      int flashFrame = (int)(shipHitTimer / 0.08f);
      if (flashFrame % 2 == 0)
        spaceship.Draw(RED);
      else
        spaceship.Draw(Fade(WHITE, 0.0f));
    } else {
      // Slower ghost flicker for remaining immunity time
      int flickerFrame = (int)(shipImmunityTimer / 0.15f);
      if (flickerFrame % 2 == 0)
        spaceship.Draw(Fade(WHITE, 0.4f));
      else
        spaceship.Draw(WHITE);
    }
  } else {
    spaceship.Draw();
  }
  for (auto &laser : spaceship.lasers)
    laser.Draw();
  for (auto &alien : aliens)
    alien.Draw();
  for (auto &laser : alienLaser)
    laser.Draw();
  for (auto &obstacle : obstacles)
    obstacle.Draw();
  drawUfo();

  // HUD
  DrawLineEx({10, float(GetScreenHeight() - 50)},
             {float(GetScreenWidth() - 10), float(GetScreenHeight() - 50)}, 3,
             YELLOW);
  DrawText("LIVES:", 10, GetScreenHeight() - 35, 20, YELLOW);
  float startX = 85.0f;
  for (int i = 0; i < lives; i++)
    DrawRectangle(startX + i * 30, GetScreenHeight() - 30, 20, 10, YELLOW);

  const char *diffText = "MED";
  if (difficulty == NOVICE)
    diffText = "NOV";
  else if (difficulty == EASY)
    diffText = "EASY";
  else if (difficulty == HARD)
    diffText = "HARD";
  else if (difficulty == ULTRAVIOLENCE)
    diffText = "UV";
  else if (difficulty == STALIN)
    diffText = "STALIN";
  else if (difficulty == NIGHTMARE)
    diffText = "NMARE";
  else if (difficulty == CUSTOM)
    diffText = "CUST";
  DrawText(TextFormat("%s LVL %02d", diffText, level), GetScreenWidth() - 170,
           GetScreenHeight() - 35, 20, YELLOW);

  DrawText("SCORE", 50, 20, 20, YELLOW);
  DrawText(TextFormat("%05d", score), 50, 45, 20, YELLOW);
  DrawText("HI-SCORE", GetScreenWidth() - 150, 20, 20, YELLOW);
  DrawText(TextFormat("%05d", highScore), GetScreenWidth() - 150, 45, 20,
           YELLOW);

  DrawRectangleLinesEx(
      {5, 5, float(GetScreenWidth() - 10), float(GetScreenHeight() - 55)}, 3,
      YELLOW);

  // Overlay states — drawn ON TOP of the game
  if (state == PAUSED) {
    drawPauseMenu();
  }

  if (state == GAME_OVER) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  Fade(BLACK, 0.55f));
    DrawText("GAME OVER", GetScreenWidth() / 2 - 100,
             GetScreenHeight() / 2 - 50, 40, RED);
    DrawText("Press SPACE to return to Menu", GetScreenWidth() / 2 - 140,
             GetScreenHeight() / 2 + 10, 20, WHITE);
  }
}

void Game::handleInput() {
  if (state == MENU) {
    handleMainMenuInput();
    return;
  }
  if (state == DIFFICULTY_SELECT) {
    handleDifficultyInput();
    return;
  }

  if (state == CUSTOM_MODE) {
    handleCustomMenuInput();
    return;
  }

  // ESC toggles pause from gameplay
  if (state == PLAYING && IsKeyPressed(KEY_ESCAPE)) {
    pauseSelected = 0;
    state = PAUSED;
    return;
  }
  if (state == PAUSED) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      state = PLAYING;
      return;
    }

    if (IsKeyPressed(KEY_UP))
      pauseSelected = (pauseSelected - 1 + 3) % 3;
    if (IsKeyPressed(KEY_DOWN))
      pauseSelected = (pauseSelected + 1) % 3;

    if (IsKeyPressed(KEY_ENTER)) {
      if (pauseSelected == 0) {
        state = PLAYING; // Resume
      } else if (pauseSelected == 1) {
        state = MENU; // Main menu
        pauseSelected = 0;
        aliens.clear();
        alienLaser.clear();
        spaceship.lasers.clear();
        StopSound(ufoLowSound);
        ufoActive = false;
      } else if (pauseSelected == 2) {
        shouldQuit = true; // Quit game
      }
    }
    return;
  }

  if (state == GAME_OVER) {
    if (IsKeyDown(KEY_SPACE)) {
      state = MENU;
    }
    return;
  }

  // Normal gameplay input
  if (IsKeyDown(KEY_LEFT))
    spaceship.moveLeft();
  else if (IsKeyDown(KEY_RIGHT))
    spaceship.moveRight();
  else if (IsKeyDown(KEY_SPACE)) {
    spaceship.Firelaser();
    if (!IsSoundPlaying(shootSound))
      PlaySound(shootSound);
  }
}
void Game::deleteInactiveLasers() {
  // Cleanup player lasers
  for (auto it = spaceship.lasers.begin(); it != spaceship.lasers.end();) {
    if (!it->active) {
      it = spaceship.lasers.erase(it);
    } else {
      ++it;
    }
  }

  // Cleanup alien lasers
  for (auto it = alienLaser.begin(); it != alienLaser.end();) {
    if (!it->active) {
      it = alienLaser.erase(it);
    } else {
      ++it;
    }
  }
}
std::vector<Alien> Game::CreateAliens() {
  std::vector<Alien> aliens;

  int rows = 5, cols = 11;
  float spacingX = 55.0f;
  float spacingY = 45.0f;
  float startY_frac = 0.15f;

  if (difficulty == CUSTOM) {
    rows = customSettings.rows;
    cols = customSettings.cols;
    spacingX = customSettings.alienSpacing;
    startY_frac = customSettings.gridStartY;
  } else {
    switch (difficulty) {
    case NOVICE:
      rows = 3;
      cols = 6;
      break;
    case EASY:
      rows = 4;
      cols = 8;
      break;
    case MEDIUM:
      rows = 5;
      cols = 11;
      break;
    case HARD:
      rows = 7;
      cols = 13;
      break;
    case ULTRAVIOLENCE:
      rows = 8;
      cols = 14;
      break;
    case STALIN:
      rows = 9;
      cols = 15;
      break;
    case NIGHTMARE:
      rows = 10;
      cols = 16;
      break;
    default:
      break;
    }
  }

  float gridW = (cols - 1) * spacingX;
  float startX = (GetScreenWidth() - gridW) / 2.0f;
  float startY = GetScreenHeight() * startY_frac;

  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      int alienType;
      if (row == 0)
        alienType = 3;
      else if (row <= 2)
        alienType = 2;
      else
        alienType = 1;

      float x = startX + col * spacingX;
      float y = startY + row * spacingY;
      Alien a(alienType, {x, y});

      // Override HP for custom mode
      if (difficulty == CUSTOM)
        a.hp = customSettings.alienHP;

      aliens.push_back(a);
    }
  }
  return aliens;
}
void Game::moveAlien() {
  // NEW CODE: Space Invaders block movement logic
  // We only want the aliens to move if the specific interval has passed.
  if (GetTime() - timeLastAlienMoved >= alienMoveInterval) {
    // Reset the movement timer
    timeLastAlienMoved = GetTime();

    // Calculate the new speed interval based on how many aliens are left.
    // The difficulty-dependent factors prevent aliens from speeding up too
    // fast.

    if (difficulty == CUSTOM) {
      // Custom mode: just use the fixed speed, no interpolation
      alienMoveInterval = customSettings.alienSpeed;
    } else {
      float startInterval = 0.5f;
      float endInterval = 0.1f;

      switch (difficulty) {
      case NOVICE:
        startInterval = 1.2f;
        endInterval = 0.8f;
        break;
      case EASY:
        startInterval = 0.9f;
        endInterval = 0.5f;
        break;
      case MEDIUM:
        startInterval = 0.6f;
        endInterval = 0.25f;
        break;
      case HARD:
        startInterval = 0.35f;
        endInterval = 0.08f;
        break;
      case ULTRAVIOLENCE:
        startInterval = 0.25f;
        endInterval = 0.05f;
        break;
      case STALIN:
        startInterval = 0.15f;
        endInterval = 0.03f;
        break;
      case NIGHTMARE:
        startInterval = 0.08f;
        endInterval = 0.01f;
        break;
      default:
        break;
      }

      int maxAliens = 1;
      if (difficulty == NOVICE)
        maxAliens = 3 * 6;
      else if (difficulty == EASY)
        maxAliens = 4 * 8;
      else if (difficulty == MEDIUM)
        maxAliens = 5 * 11;
      else if (difficulty == HARD)
        maxAliens = 7 * 13;
      else if (difficulty == ULTRAVIOLENCE)
        maxAliens = 8 * 14;
      else if (difficulty == STALIN)
        maxAliens = 9 * 15;
      else if (difficulty == NIGHTMARE)
        maxAliens = 10 * 16;

      float ratio = (float)aliens.size() / (float)maxAliens;
      alienMoveInterval = endInterval + (ratio * (startInterval - endInterval));
    }

    // Play movement sound
    PlaySound(moveSounds[currentMoveSound]);
    currentMoveSound = (currentMoveSound + 1) % 4;

    // Move all aliens horizontally by a "step" (e.g., 15 pixels)
    int stepSize = (difficulty == CUSTOM) ? customSettings.stepSize : 15;
    for (auto &alien : aliens) {
      alien.Update(alienDirection * stepSize);
    }

    // Now check if ANY alien in the grid has hit the screen edge
    bool hitEdge = false;
    for (auto &alien : aliens) {
      if (alien.position.x + alien.alienImages[alien.type - 1].width >=
              GetScreenWidth() ||
          alien.position.x <= 0) {
        hitEdge = true;
        break; // Stop checking, we already know the block hit the edge
      }
    }

    // If the block hit the edge, we need to reverse direction and move them
    // down
    if (hitEdge) {
      alienDirection *= -1; // Flip horizontal direction
      moveAliensDown((difficulty == CUSTOM) ? customSettings.descentSpeed : 15);

      // Step the aliens back away from the edge so they don't get stuck
      // triggering the collision over and over again on the next frame
      for (auto &alien : aliens) {
        alien.Update(alienDirection * stepSize);
      }
    }
  }
}

void Game::moveAliensDown(int distance) {
  for (auto &alien : aliens) {
    alien.position.y += distance;
  }
}

// I do not understand this, this was generated by an llm, but it works (smh)
std::vector<Obstacle> Game::createObstacles() {
  const int obstacleCount =
      (difficulty == CUSTOM) ? customSettings.shieldCount : 4;
  // If zero shields, return empty
  if (obstacleCount == 0)
    return {};

  const int blockSize = 3;
  int obstacleWidth =
      Obstacle::grid[0].size() * blockSize; // width in pixels (columns * 3)
  int totalObstaclesWidth = obstacleCount * obstacleWidth;
  float availableSpace = GetScreenWidth() - totalObstaclesWidth;
  float gap =
      availableSpace / (obstacleCount + 1); // gaps before, between, after

  std::vector<Obstacle> obsVector;
  for (int i = 0; i < obstacleCount; i++) {
    float offsetX = gap + i * (obstacleWidth + gap);
    float posY = float(GetScreenHeight() - 150);
    obsVector.push_back(Obstacle({offsetX, posY}));
  }
  return obsVector;
}

void Game::alienShootLaser() {
  // NEW CODE: Shoot on a timer, not every single frame!
  // We also check !aliens.empty() to prevent crashing if all aliens are dead
  if (GetTime() - timeLastAlienShot >= alienShootInterval && !aliens.empty()) {
    // Reset timer
    timeLastAlienShot = GetTime();

    // Randomize the interval for the next shot (between 0.5 and 2.5 seconds)
    // This makes the alien fire rate unpredictable like the arcade game,
    // change this to increase or decrease the fire rate
    float scale = 1.0f;

    if (difficulty == NOVICE)
      scale = 3.0f;
    if (difficulty == EASY)
      scale = 2.0f;
    if (difficulty == HARD)
      scale = 0.5f;
    if (difficulty == ULTRAVIOLENCE)
      scale = 0.3f;
    if (difficulty == STALIN)
      scale = 0.15f;
    if (difficulty == NIGHTMARE)
      scale = 0.05f;

    float customScale =
        (difficulty == CUSTOM) ? customSettings.fireRateScale : 1.0f;
    alienShootInterval = (GetRandomValue(1, 6) / 10.0) * scale * customScale;

    // Pick a random alien to shoot
    int randomIndex = GetRandomValue(0, aliens.size() - 1);
    Alien &alien = aliens[randomIndex];

    // Fire laser from the center bottom of that alien, passing the alien's
    // damage stat
    int typeIdx = alien.type - 1;
    if (typeIdx < 0)
      typeIdx = 0;
    if (typeIdx > 2)
      typeIdx = 2;

    alienLaser.push_back(
        Laser({alien.position.x + Alien::alienImages[typeIdx].width / 2.0f,
               alien.position.y + Alien::alienImages[typeIdx].height},
              6, alien.damage));
  }
}
void Game::checkForCollisions() {

  // --- SPACESHIP LASER COLLISIONS ---
  for (auto &laser : spaceship.lasers) {

    // 1. Check collision with Aliens
    auto alienIt = aliens.begin();
    while (alienIt != aliens.end()) {
      if (CheckCollisionRecs(alienIt->getRect(), laser.getRect())) {
        alienIt->hp -= 1;
        laser.active = false;
        if (alienIt->hp <= 0) {
          if (alienIt->type == 3)
            score +=
                int(30 * (difficulty == CUSTOM ? customSettings.scoreMultiplier
                                               : 1.0f));
          else if (alienIt->type == 2)
            score +=
                int(20 * (difficulty == CUSTOM ? customSettings.scoreMultiplier
                                               : 1.0f));
          else
            score +=
                int(10 * (difficulty == CUSTOM ? customSettings.scoreMultiplier
                                               : 1.0f));
          if (score > highScore)
            highScore = score;
          PlaySound(alienDeathSound);
          alienIt = aliens.erase(alienIt);
        } else {
          ++alienIt;
        }
        break;
      } else {
        ++alienIt;
      }
    }

    // 2. Check collision with UFO
    if (laser.active && ufoActive) {
      Rectangle ufoRect = {ufoPosition.x, ufoPosition.y,
                           (float)ufoTexture.width, (float)ufoTexture.height};
      if (CheckCollisionRecs(ufoRect, laser.getRect())) {
        ufoActive = false;
        laser.active = false;
        score += 100;
        if (score > highScore)
          highScore = score;
        PlaySound(ufoHighSound);
        StopSound(ufoLowSound);
      }
    }

    // 3. Check collision with Obstacles
    if (laser.active) {
      for (auto &obstacle : obstacles) {
        float extraWidth = (laser.damage / 10.0f) * 2.0f;
        Rectangle damageRect = {
            laser.getRect().x - extraWidth, laser.getRect().y,
            laser.getRect().width + (extraWidth * 2), laser.getRect().height};
        auto blockIt = obstacle.blocks.begin();
        while (blockIt != obstacle.blocks.end()) {
          if (CheckCollisionRecs(blockIt->getRect(), damageRect)) {
            blockIt = obstacle.blocks.erase(blockIt);
            laser.active = false;
            break;
          } else {
            ++blockIt;
          }
        }
        if (!laser.active)
          break;
      }
    }
  }

  // --- ALIEN LASER COLLISIONS ---
  for (auto &laser : alienLaser) {

    // 4. Check collision with Spaceship
    if (laser.active &&
        CheckCollisionRecs(laser.getRect(), spaceship.getRect())) {
      laser.active = false;
      if (shipImmunityTimer <= 0.0f) {
        lives--;
        PlaySound(explosionSound);
        shipHit = true;
        shipHitTimer = shipHitDuration;
        shipImmunityTimer = 3.0f;
        if (lives <= 0)
          gameOver();
      }
    }

    // 5. Check collision with Obstacles
    if (laser.active) {
      for (auto &obstacle : obstacles) {
        float extraWidth = (laser.damage / 10.0f) * 2.0f;
        Rectangle damageRect = {
            laser.getRect().x - extraWidth, laser.getRect().y,
            laser.getRect().width + (extraWidth * 2), laser.getRect().height};
        auto blockIt = obstacle.blocks.begin();
        while (blockIt != obstacle.blocks.end()) {
          if (CheckCollisionRecs(blockIt->getRect(), damageRect)) {
            blockIt = obstacle.blocks.erase(blockIt);
            laser.active = false;
          } else {
            ++blockIt;
          }
        }
        if (!laser.active)
          break;
      }
    }
  }

  // --- ALIEN BODY COLLISIONS ---
  for (auto &alien : aliens) {

    // 6. Alien vs Obstacles
    for (auto &obstacle : obstacles) {
      auto blockIt = obstacle.blocks.begin();
      while (blockIt != obstacle.blocks.end()) {
        if (CheckCollisionRecs(alien.getRect(), blockIt->getRect()))
          blockIt = obstacle.blocks.erase(blockIt);
        else
          ++blockIt;
      }
    }

    // 7. Alien vs Spaceship
    if (CheckCollisionRecs(alien.getRect(), spaceship.getRect())) {
      if (shipImmunityTimer <= 0.0f) {
        PlaySound(explosionSound);
        gameOver();
      }
    }
  }
}

void Game::gameOver() { state = GAME_OVER; }

void Game::Reset() {
  if (difficulty == CUSTOM) {
    resetCustom();
    return;
  }
  // Clear all vectors
  aliens.clear();
  alienLaser.clear();
  spaceship.lasers.clear();
  obstacles.clear();

  // Re-initialize
  aliens = CreateAliens();
  obstacles = createObstacles();
  lives = 3;
  score = 0;
  level = 1;
  alienDirection = 1;
  // Re-initialize parameters based on difficulty
  switch (difficulty) {
    // Reset() base intervals
  case NOVICE:
    alienMoveInterval = 1.2;
    lives = 10;
    spaceship.setLaserSpeed(-18);
    break;
  case EASY:
    alienMoveInterval = 0.9;
    lives = 5;
    spaceship.setLaserSpeed(-16);
    break;
  case MEDIUM:
    alienMoveInterval = 0.6;
    lives = 3;
    spaceship.setLaserSpeed(-20);
    break;
  case HARD:
    alienMoveInterval = 0.35;
    lives = 2;
    spaceship.setLaserSpeed(-22);
    break;
  case ULTRAVIOLENCE:
    alienMoveInterval = 0.25;
    lives = 2;
    spaceship.setLaserSpeed(-24);
    break;
  case STALIN:
    alienMoveInterval = 0.15;
    lives = 1;
    spaceship.setLaserSpeed(-25);
    break;
  case NIGHTMARE:
    alienMoveInterval = 0.08;
    lives = 1;
    spaceship.setLaserSpeed(-28);
    break;
  }

  timeLastAlienMoved = GetTime();
  timeLastAlienShot = GetTime();
  alienShootInterval = GetRandomValue(10, 30) / 10.0;
  if (difficulty == STALIN)
    alienShootInterval = 0.1;
  currentMoveSound = 0;
  ufoActive = false;
  nextUfoSpawnTime = GetTime() + GetRandomValue(10, 20);
  StopSound(ufoLowSound);

  // Reset spaceship position and power-ups
  spaceship.Reset();
  spaceship.setLaserSpeed(-15);
  spaceship.setLaserDamage(1);
}

void Game::spawnUfo() {
  ufoActive = true;
  ufoDirection = (GetRandomValue(0, 1) == 0) ? 1 : -1;
  if (ufoDirection == 1) {
    ufoPosition = {-float(ufoTexture.width), 30};
  } else {
    ufoPosition = {float(GetScreenWidth()), 30};
  }
  nextUfoSpawnTime = GetTime() + GetRandomValue(15, 30);
  PlaySound(ufoLowSound);
}

void Game::updateUfo() {
  if (!ufoActive) {
    if (GetTime() >= nextUfoSpawnTime) {
      spawnUfo();
    }
    return;
  }

  ufoPosition.x += ufoDirection * 2.0f;

  if (ufoPosition.x < -ufoTexture.width || ufoPosition.x > GetScreenWidth()) {
    ufoActive = false;
    StopSound(ufoLowSound);
  }
}

void Game::drawUfo() {
  if (ufoActive) {
    DrawTextureV(ufoTexture, ufoPosition, WHITE);
  }
}

void Game::drawMenu() {
  DrawText("STALIN INVADERS", GetScreenWidth() / 2 - 180, 150, 40, YELLOW);
  DrawText("SELECT DIFFICULTY:", GetScreenWidth() / 2 - 120, 250, 20, WHITE);
  DrawText("[1] NOVICE", GetScreenWidth() / 2 - 60, 300, 20, BLUE);
  DrawText("[2] EASY", GetScreenWidth() / 2 - 60, 340, 20, GREEN);
  DrawText("[3] MEDIUM", GetScreenWidth() / 2 - 60, 380, 20, YELLOW);
  DrawText("[4] HARD", GetScreenWidth() / 2 - 60, 420, 20, ORANGE);
  DrawText("[5] STALIN", GetScreenWidth() / 2 - 60, 460, 20, RED);
}

void Game::handleMenuInput() {
  if (IsKeyPressed(KEY_ONE)) {
    difficulty = NOVICE;
    Reset();
    state = PLAYING;
  } else if (IsKeyPressed(KEY_TWO)) {
    difficulty = EASY;
    Reset();
    state = PLAYING;
  } else if (IsKeyPressed(KEY_THREE)) {
    difficulty = MEDIUM;
    Reset();
    state = PLAYING;
  } else if (IsKeyPressed(KEY_FOUR)) {
    difficulty = HARD;
    Reset();
    state = PLAYING;
  } else if (IsKeyPressed(KEY_FIVE)) {
    difficulty = STALIN;
    Reset();
    state = PLAYING;
  }
}

void Game::checkLevelCompletion() {
  if (aliens.empty()) {
    nextLevel();
  }
}

void Game::nextLevel() {
  level++;
  aliens = CreateAliens();
  obstacles = createObstacles();
  alienLaser.clear();
  spaceship.lasers.clear();
  ufoActive = false;
  alienDirection = 1;
  StopSound(ufoLowSound);

  // Increase speed slightly based on level
  float scl = (difficulty == CUSTOM) ? customSettings.levelSpeedScale : 0.9f;
  alienMoveInterval *= scl;

  if (alienMoveInterval < 0.1)
    alienMoveInterval = 0.1;
}

// ─── MENU STARS (animated background)
// ────────────────────────────────────────

void Game::initMenuStars() {
  for (int i = 0; i < 80; i++) {
    auto &s = menuStars[i];
    s.x = GetRandomValue(0, GetScreenWidth());
    s.y = GetRandomValue(0, GetScreenHeight());
    s.drift = GetRandomValue(-5, 5) / 100.0f;

    if (i < 50) {
      s.speed = GetRandomValue(20, 40) / 100.0f;
      s.size = 1.0f;
    } else if (i < 70) {
      s.speed = GetRandomValue(80, 120) / 100.0f;
      s.size = 1.5f;
    } else {
      s.speed = GetRandomValue(180, 220) / 100.0f;
      s.size = 2.0f;
    }
  }
}

void Game::updateMenuStars() {
  for (int i = 0; i < 80; i++) {
    auto &s = menuStars[i];
    s.y += s.speed;
    s.x += s.drift;
    if (s.y > GetScreenHeight()) {
      s.y = 0;
      s.x = GetRandomValue(0, GetScreenWidth());
    }
    if (s.x < 0)
      s.x = GetScreenWidth();
    if (s.x > GetScreenWidth())
      s.x = 0;
  }
}
void Game::drawMenuStars() {
  for (int i = 0; i < 80; i++) {
    auto &s = menuStars[i];
    Color c;
    if (i < 50)
      c = Fade(WHITE, 0.4f);
    else if (i < 70)
      c = Fade(WHITE, 0.75f);
    else
      c = WHITE;
    DrawCircleV({s.x, s.y}, s.size, c);
  }
}

// ─── MAIN MENU
// ────────────────────────────────────────────────────────────────

void Game::drawMainMenu() {
  int W = GetScreenWidth();
  int H = GetScreenHeight();

  updateMenuStars();
  drawMenuStars();

  // Outer border
  DrawRectangleLinesEx({5, 5, float(W - 10), float(H - 10)}, 3, YELLOW);

  // Title
  const char *title = "STALIN";
  const char *subtitle = "I N V A D E R S";
  int titleSize = W / 10;
  int subSize = W / 30;

  int titleX = W / 2 - MeasureText(title, titleSize) / 2;
  DrawText(title, titleX, H * 0.18f, titleSize, YELLOW);

  int subX = W / 2 - MeasureText(subtitle, subSize) / 2;
  DrawText(subtitle, subX, H * 0.18f + titleSize + 8, subSize, RED);

  // Separator line
  DrawLineEx({float(W) * 0.2f, H * 0.42f}, {float(W) * 0.8f, H * 0.42f}, 2,
             YELLOW);

  // Blinking "PRESS ENTER"
  if ((int)(GetTime() * 2) % 2 == 0) {
    const char *prompt = "PRESS ENTER TO START";
    int promptSize = W / 40;
    DrawText(prompt, W / 2 - MeasureText(prompt, promptSize) / 2, H * 0.55f,
             promptSize, WHITE);
  }

  // Q to quit
  const char *quitText = "PRESS Q TO QUIT";
  int quitSize = W / 55;
  DrawText(quitText, W / 2 - MeasureText(quitText, quitSize) / 2, H * 0.65f,
           quitSize, DARKGRAY);

  // Controls hint
  const char *controls = "ARROWS: MOVE    SPACE: FIRE";
  int ctrlSize = W / 55;
  DrawText(controls, W / 2 - MeasureText(controls, ctrlSize) / 2, H * 0.72f,
           ctrlSize, GRAY);

  // Footer
  const char *footer = "University of Gujrat  |  BS IT  |  2026";
  int footSize = W / 70;
  DrawText(footer, W / 2 - MeasureText(footer, footSize) / 2, H - 35, footSize,
           DARKGRAY);
}

// ─── handleMainMenuInput ─────────────────────────────────────
//
//  Mouse hit zones are matched to the text positions in
//  drawMainMenu(). Using fractional Y so they stay aligned
//  if the window is resized.
//
void Game::handleMainMenuInput() {
  int W = GetScreenWidth();
  int H = GetScreenHeight();
  Vector2 mouse = GetMousePosition();

  // // "PRESS ENTER TO START" blinking text area
  // Rectangle startRect = {float(W) * 0.2f, float(H) * 0.52f, float(W) * 0.6f,
  //                        40};
  // if (CheckCollisionPointRec(mouse, startRect) &&
  //     IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
  //   selectedDifficulty = 2; // default to MEDIUM
  //   state = DIFFICULTY_SELECT;
  //   return;
  // }

  // // "PRESS Q TO QUIT" text area
  // Rectangle quitRect = {float(W) * 0.2f, float(H) * 0.62f, float(W) * 0.6f, 35};
  // if (CheckCollisionPointRec(mouse, quitRect) &&
  //     IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
  //   shouldQuit = true;
  //   return;
  // }

  // Keyboard fallbacks
  if (IsKeyPressed(KEY_Q)) {
    shouldQuit = true;
    return;
  }
  
  if (IsKeyPressed(KEY_ENTER)) {
    selectedDifficulty = 2;
    state = DIFFICULTY_SELECT;
    return;
  }
}

// ─── DIFFICULTY SELECT
// ────────────────────────────────────────────────────────

void Game::drawDifficultyMenu() {
  int W = GetScreenWidth();
  int H = GetScreenHeight();

  updateMenuStars();
  drawMenuStars();

  DrawRectangleLinesEx({5, 5, float(W - 10), float(H - 10)}, 3, YELLOW);

  // Title
  const char *title = "SELECT DIFFICULTY";
  int titleSize = W / 25;
  DrawText(title, W / 2 - MeasureText(title, titleSize) / 2, H * 0.08f,
           titleSize, YELLOW);

  DrawLineEx({float(W) * 0.1f, H * 0.20f}, {float(W) * 0.9f, H * 0.20f}, 2,
             YELLOW);

  // Difficulty data
  struct DiffOption {
    const char *name;
    const char *desc;
    const char *stats;
    Color color;
  };

  DiffOption opts[8] = {
      {"NOVICE", "I'm Too Young To Die", "10 LIVES | GLACIAL  |  3x6  ", GREEN},
      {"EASY", "Hey, Not Too Rough", " 5 LIVES | SLOW     |  4x8  ", SKYBLUE},
      {"MEDIUM", "Hurt Me Plenty", " 3 LIVES | CLASSIC  |  5x11 ", YELLOW},
      {"HARD", "Ultra-Violence", " 2 LIVES | RUTHLESS |  7x13 ", ORANGE},
      {"ULTRA-VIOLENCE",
       "No Rest For The Living",
       " 2 LIVES | BRUTAL   |  8x14 ",
       {255, 120, 0, 255}},
      {"STALIN", "Nightmare!", " 1 LIFE  | CHAOS    |  9x15 ", RED},
      {"NIGHTMARE",
       "You Will Not Finish This",
       " 1 LIFE  | INSANITY | 10x16 ",
       {180, 0, 255, 255}},
      {"CUSTOM",
       "Build Your Own Hell",
       "? LIVES | ?        |  ?x?  ",
       {0, 200, 255, 255}}};
  // Card layout
  float cardH = H * 0.085f;
  float cardW = W * 0.65f;
  float cardX = (W - cardW) / 2.0f;
  float totalH = 7 * cardH + 6 * 8;
  float startY = H * 0.22f; // starts just below the separator line
  for (int i = 0; i < 8; i++) {
    float y = startY + i * (cardH + 12);
    bool selected = (i == selectedDifficulty);
    Color c = opts[i].color;

    // Card background
    DrawRectangleRounded({cardX, y, cardW, cardH}, 0.15f, 8,
                         selected ? Fade(c, 0.25f) : Fade(c, 0.07f));
    // Card border
    DrawRectangleRoundedLines({cardX, y, cardW, cardH}, 0.15f, 8,
                              selected ? c : Fade(c, 0.35f));

    // Selection arrow
    if (selected)
      DrawText(">", cardX - 28, y + cardH / 2 - 12, 24, c);

    // Name
    int nameSize = (int)(cardH * 0.38f);
    DrawText(opts[i].name, cardX + 20, y + cardH * 0.12f, nameSize,
             selected ? c : Fade(c, 0.6f));

    // Desc
    int descSize = (int)(cardH * 0.22f);
    DrawText(opts[i].desc, cardX + 20, y + cardH * 0.55f, descSize,
             selected ? WHITE : GRAY);

    // Stats (right-aligned inside card)
    int statSize = (int)(cardH * 0.20f);
    int statW = MeasureText(opts[i].stats, statSize);
    DrawText(opts[i].stats, cardX + cardW - statW - 20, y + cardH * 0.55f,
             statSize, selected ? c : Fade(c, 0.5f));
  }

  // Bottom hint
  const char *hint = "UP/DOWN: Navigate    ENTER: Confirm    BACKSPACE: Back";
  int hintSize = W / 65;
  DrawText(hint, W / 2 - MeasureText(hint, hintSize) / 2, H - 35, hintSize,
           DARKGRAY);
}

// ─── handleDifficultyInput ───────────────────────────────────
//
//  Mouse hover over a card updates selectedDifficulty in real
//  time so the highlight follows the cursor — same feedback
//  the keyboard gives. A click then confirms the choice.
//
//  Card geometry must match drawDifficultyMenu exactly.
//
void Game::handleDifficultyInput() {
  int W = GetScreenWidth();
  int H = GetScreenHeight();
  Vector2 mouse = GetMousePosition();

  float cardH = H * 0.085f;
  float cardW = W * 0.65f;
  float cardX = (W - cardW) / 2.0f;
  float startY = H * 0.22f;

  for (int i = 0; i < 8; i++) {
    float y = startY + i * (cardH + 12);
    Rectangle cardRect = {cardX, y, cardW, cardH};
    if (!CheckCollisionPointRec(mouse, cardRect))
      continue;

    // Hover highlights the card
    selectedDifficulty = i;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      if (i == 7) {
        // CUSTOM card — go to custom mode screen instead of playing
        customSelected = 0;
        customPage = 0;
        state = CUSTOM_MODE;
      } else {
        difficulty = (Difficulty)i;
        Reset();
        state = PLAYING;
      }
    }
  }

  // ── Keyboard ───────────────────────────────────────────────
  if (IsKeyPressed(KEY_UP))
    selectedDifficulty = (selectedDifficulty - 1 + 8) % 8;
  if (IsKeyPressed(KEY_DOWN))
    selectedDifficulty = (selectedDifficulty + 1) % 8;
  if (IsKeyPressed(KEY_BACKSPACE))
    state = MENU;

  if (IsKeyPressed(KEY_ENTER)) {
    if (selectedDifficulty == 7) {
      customSelected = 0;
      customPage = 0;
      state = CUSTOM_MODE;
    } else {
      difficulty = (Difficulty)selectedDifficulty;
      Reset();
      state = PLAYING;
    }
  }

  // Number key shortcuts — preserved from original code
  if (IsKeyPressed(KEY_ONE)) {
    difficulty = NOVICE;
    Reset();
    state = PLAYING;
  }
  if (IsKeyPressed(KEY_TWO)) {
    difficulty = EASY;
    Reset();
    state = PLAYING;
  }
  if (IsKeyPressed(KEY_THREE)) {
    difficulty = MEDIUM;
    Reset();
    state = PLAYING;
  }
  if (IsKeyPressed(KEY_FOUR)) {
    difficulty = HARD;
    Reset();
    state = PLAYING;
  }
  if (IsKeyPressed(KEY_FIVE)) {
    difficulty = STALIN;
    Reset();
    state = PLAYING;
  }
  if (IsKeyPressed(KEY_SIX)) {
    difficulty = ULTRAVIOLENCE;
    Reset();
    state = PLAYING;
  }
  if (IsKeyPressed(KEY_SEVEN)) {
    difficulty = NIGHTMARE;
    Reset();
    state = PLAYING;
  }
  if (IsKeyPressed(KEY_C)) {
    customSelected = 0;
    customPage = 0;
    state = CUSTOM_MODE;
  }
}

// ============================================================
//  CUSTOM MODE — paste this entire block into game.cpp
//  Place it just before (or after) your drawDifficultyMenu()
// ============================================================

#include <cmath>
#include <cstring>
#include <fstream>

// ---- Preset file path ----
static const char *PRESET_FILE = "assets/presets.dat";
static const uint32_t PRESET_MAGIC = 0x5354414C; // "STAL"

// -----------------------------------------------------------
//  savePresets / loadPresets  (binary)
// -----------------------------------------------------------
void Game::savePresets() {
  std::ofstream f(PRESET_FILE, std::ios::binary);
  if (!f)
    return;
  f.write(reinterpret_cast<const char *>(&PRESET_MAGIC), 4);
  f.write(reinterpret_cast<const char *>(&presetCount), sizeof(int));
  f.write(reinterpret_cast<const char *>(presets),
          sizeof(CustomPreset) * presetCount);
}

void Game::loadPresets() {
  std::ifstream f(PRESET_FILE, std::ios::binary);
  if (!f) {
    presetCount = 0;
    return;
  }
  uint32_t magic = 0;
  f.read(reinterpret_cast<char *>(&magic), 4);
  if (magic != PRESET_MAGIC) {
    presetCount = 0;
    return;
  }
  f.read(reinterpret_cast<char *>(&presetCount), sizeof(int));
  if (presetCount < 0 || presetCount > MAX_PRESETS) {
    presetCount = 0;
    return;
  }
  f.read(reinterpret_cast<char *>(presets), sizeof(CustomPreset) * presetCount);
}

// -----------------------------------------------------------
//  resetCustom  —  like Reset() but driven by customSettings
// -----------------------------------------------------------
void Game::resetCustom() {
  aliens.clear();
  alienLaser.clear();
  spaceship.lasers.clear();
  obstacles.clear();

  difficulty = CUSTOM;
  level = customSettings.startingLevel;
  lives = customSettings.lives;
  score = 0;
  alienDirection = 1;
  alienMoveInterval = customSettings.alienSpeed;

  aliens = CreateAliens();       // reads customSettings when difficulty==CUSTOM
  obstacles = createObstacles(); // reads customSettings.shieldCount

  spaceship.Reset();
  spaceship.setLaserSpeed(customSettings.laserSpeed);
  spaceship.setLaserDamage(customSettings.laserDamage);

  timeLastAlienMoved = GetTime();
  timeLastAlienShot = GetTime();
  alienShootInterval =
      (GetRandomValue(10, 30) / 10.0) * customSettings.fireRateScale;
  currentMoveSound = 0;
  ufoActive = false;
  nextUfoSpawnTime = GetTime() + customSettings.ufoFrequency;
  StopSound(ufoLowSound);
}

// -----------------------------------------------------------
//  drawCustomMenu
// -----------------------------------------------------------
// ─── drawCustomMenu ──────────────────────────────────────────
//
//  Layout contract (top to bottom, all in screen-fraction units):
//    0.03  Title text
//    0.09  Page indicator ("PAGE 1/2")
//    0.125 Separator line  ← was colliding with page text before
//    0.14  Cards start     ← pushed down to clear separator
//    0.93  PLAY/BACK buttons (fraction instead of fixed px so they
//          don't get clipped on smaller screens)
//
//  The preset naming overlay is drawn LAST so it sits on top of
//  everything. It dims the whole screen first so the dialog feels
//  modal rather than floating.
//
void Game::drawCustomMenu() {
  int W = GetScreenWidth();
  int H = GetScreenHeight();

  updateMenuStars();
  drawMenuStars();
  DrawRectangleLinesEx({5, 5, float(W - 10), float(H - 10)}, 3, YELLOW);

  // ── Title ──────────────────────────────────────────────────
  const char *title = "CUSTOM MODE";
  int titleSz = W / 25;
  DrawText(title, W / 2 - MeasureText(title, titleSz) / 2, H * 0.03f, titleSz,
           YELLOW);

  // Page indicator sits below the title, left-aligned to match
  // the card column so it reads as part of that column.
  const char *pageStr = (customPage == 0) ? "PAGE 1/2  (TAB for next)"
                                          : "PAGE 2/2  (TAB for prev)";
  DrawText(pageStr, W * 0.04f, H * 0.09f, 16, YELLOW);

  // Separator only spans the card column width, not full screen,
  // so the preset panel on the right has its own visual border.
  DrawLineEx({float(W) * 0.04f, H * 0.125f}, {float(W) * 0.62f, H * 0.125f}, 1,
             Fade(YELLOW, 0.5f));

  // ── Parameter list ─────────────────────────────────────────
  // Each page holds a subset of CustomSettings fields.
  // We use a local Param struct so draw code stays generic —
  // every param is just a label, current value, and range.
  struct Param {
    const char *label;
    const char *unit;
    float value;
    float minV, maxV, step;
  };

  Param page0[9] = {
      {"Rows", "", float(customSettings.rows), 1, 15, 1},
      {"Columns", "", float(customSettings.cols), 1, 20, 1},
      {"Lives", "", float(customSettings.lives), 1, 20, 1},
      {"Alien Speed", "s", customSettings.alienSpeed, 0.05f, 2.0f, 0.05f},
      {"Fire Rate Scale", "x", customSettings.fireRateScale, 0.1f, 5.0f, 0.1f},
      // laserSpeed is stored negative (upward), displayed positive
      {"Laser Speed", "", float(-customSettings.laserSpeed), 5, 50, 1},
      {"Laser Damage", "", float(customSettings.laserDamage), 1, 10, 1},
      {"Alien HP", "", float(customSettings.alienHP), 1, 5, 1},
      {"Step Size", "px", float(customSettings.stepSize), 5, 40, 1},
  };
  Param page1[8] = {
      {"Descent Speed", "px", float(customSettings.descentSpeed), 5, 40, 1},
      {"Grid Start Y", "%", customSettings.gridStartY * 100, 5, 50, 1},
      {"Alien Spacing", "px", customSettings.alienSpacing, 30, 80, 1},
      {"Starting Level", "", float(customSettings.startingLevel), 1, 20, 1},
      {"UFO Frequency", "s", customSettings.ufoFrequency, 5, 60, 1},
      {"Level Spd Scale", "x", customSettings.levelSpeedScale, 0.5f, 1.0f,
       0.05f},
      {"Shields", "", float(customSettings.shieldCount), 0, 6, 1},
      {"Score Mult", "x", customSettings.scoreMultiplier, 1.0f, 5.0f, 0.5f},
  };

  int paramCount = (customPage == 0) ? 9 : 8;
  Param *params = (customPage == 0) ? page0 : page1;

  float cardW = W * 0.58f;
  float cardX = W * 0.04f;
  float cardH = H * 0.072f;
  float startY = H * 0.14f; // cleared below separator

  for (int i = 0; i < paramCount; i++) {
    float y = startY + i * (cardH + 6);
    bool sel = (customSelected == i);
    Color c = sel ? YELLOW : Fade(WHITE, 0.5f);

    DrawRectangleRounded({cardX, y, cardW, cardH}, 0.15f, 6,
                         sel ? Fade(YELLOW, 0.12f) : Fade(WHITE, 0.04f));
    DrawRectangleRoundedLines({cardX, y, cardW, cardH}, 0.15f, 6,
                              sel ? YELLOW : Fade(WHITE, 0.2f));

    int labelSz = (int)(cardH * 0.38f);
    DrawText(params[i].label, cardX + 14, y + cardH * 0.30f, labelSz, c);

    // Progress bar shows where the value sits within [minV, maxV]
    float barX = cardX + cardW * 0.52f;
    float barW = cardW * 0.42f;
    float ratio =
        (params[i].value - params[i].minV) / (params[i].maxV - params[i].minV);
    ratio = fmaxf(0.0f, fminf(1.0f, ratio));
    DrawRectangle(barX, y + cardH * 0.55f, barW, cardH * 0.18f,
                  Fade(WHITE, 0.1f));
    DrawRectangle(barX, y + cardH * 0.55f, barW * ratio, cardH * 0.18f,
                  sel ? YELLOW : Fade(YELLOW, 0.4f));

    char valStr[32];
    if (params[i].step < 1.0f)
      snprintf(valStr, sizeof(valStr), "%.2f%s", params[i].value,
               params[i].unit);
    else
      snprintf(valStr, sizeof(valStr), "%.0f%s", params[i].value,
               params[i].unit);

    int valSz = labelSz;
    DrawText(valStr, barX - MeasureText(valStr, valSz) - 8, y + cardH * 0.25f,
             valSz, c);

    // Arrow hints only on the selected row
    if (sel) {
      DrawText("<", barX - MeasureText(valStr, valSz) - 24, y + cardH * 0.25f,
               valSz, YELLOW);
      DrawText(">", barX + barW + 6, y + cardH * 0.25f, valSz, YELLOW);
    }
  }

  // ── PLAY / BACK buttons ────────────────────────────────────
  // Using H * 0.93f instead of H - 55 so buttons scale with
  // window height and never get clipped on shorter screens.
  Vector2 mouse = GetMousePosition();

  Rectangle playRect = {cardX + cardW - 120, float(H) - 45, 120, 30};
  bool playHov = CheckCollisionPointRec(mouse, playRect);
  DrawRectangleRounded(playRect, 0.2f, 4,
                       playHov ? Fade(GREEN, 0.3f) : Fade(GREEN, 0.08f));
  DrawRectangleRoundedLines(playRect, 0.2f, 4,
                            playHov ? GREEN : Fade(GREEN, 0.4f));
  DrawText("PLAY", playRect.x + 38, playRect.y + 8, 18,
           playHov ? GREEN : Fade(GREEN, 0.7f));
  Rectangle backRect = {cardX, float(H) - 45, 100, 30};

  bool backHov = CheckCollisionPointRec(mouse, backRect);
  DrawRectangleRounded(backRect, 0.2f, 4,
                       backHov ? Fade(RED, 0.3f) : Fade(RED, 0.08f));
  DrawRectangleRoundedLines(backRect, 0.2f, 4, backHov ? RED : Fade(RED, 0.4f));
  DrawText("BACK", backRect.x + 22, backRect.y + 8, 18,
           backHov ? RED : Fade(RED, 0.7f));

  // ── Preset panel + hint text ───────────────────────────────
  drawPresetPanel();

  const char *hint = "LEFT/RIGHT: Change  UP/DOWN: Navigate  "
                     "TAB: Page  R: Random  ENTER: Play  BACKSPACE: Back";
  int hintSz = W / 80;
  DrawText(hint, W / 2 - MeasureText(hint, hintSz) / 2, H - 22, hintSz,
           Fade(DARKGRAY, 0.8f));

  // ── Preset naming overlay ──────────────────────────────────
  // Drawn absolutely last so it covers everything.
  // The full-screen dim makes it feel modal (user knows input
  // is captured here and nowhere else).
  if (namingPreset) {
    DrawRectangle(0, 0, W, H, Fade(BLACK, 0.82f));

    float bW = W * 0.45f, bH = H * 0.22f;
    float bX = (W - bW) / 2.0f, bY = (H - bH) / 2.0f;
    DrawRectangleRounded({bX, bY, bW, bH}, 0.12f, 8, {18, 18, 18, 255});
    DrawRectangleRoundedLines({bX, bY, bW, bH}, 0.12f, 8, YELLOW);

    const char *prompt = "NAME THIS PRESET";
    int pSz = W / 40;
    DrawText(prompt, W / 2 - MeasureText(prompt, pSz) / 2, bY + 18, pSz,
             YELLOW);
    DrawText("ENTER to save   ESC to cancel",
             W / 2 - MeasureText("ENTER to save   ESC to cancel", 14) / 2,
             bY + 18 + pSz + 6, 14, Fade(WHITE, 0.5f));

    float ibX = bX + 20, ibY = bY + bH * 0.55f, ibW = bW - 40, ibH = 34;
    DrawRectangleRounded({ibX, ibY, ibW, ibH}, 0.15f, 4, Fade(WHITE, 0.08f));
    DrawRectangleRoundedLines({ibX, ibY, ibW, ibH}, 0.15f, 4, YELLOW);

    char display[34];
    snprintf(display, sizeof(display), "%s_", presetNameBuf);
    DrawText(display, ibX + 10, ibY + 7, 20, WHITE);
  }
}

// ─── drawPresetPanel ─────────────────────────────────────────
//
//  The panel is purely a draw function — all click handling
//  happens here too (draw + interact in one place keeps the
//  rect calculations from drifting out of sync between draw
//  and input code).
//
//  Layout inside the panel:
//    pY + 8   "PRESETS" title
//    pY + 35  separator line
//    pY + 45  preset rows (36px each)
//    pY+pH-38 SAVE PRESET button
//
void Game::drawPresetPanel() {
  int W = GetScreenWidth();
  int H = GetScreenHeight();

  float pX = W * 0.65f;
  float pW = W * 0.31f;
  float pY = H * 0.14f;
  float pH = H * 0.65f; // was 0.72f

  DrawRectangleRounded({pX, pY, pW, pH}, 0.08f, 8, Fade(WHITE, 0.04f));
  DrawRectangleRoundedLines({pX, pY, pW, pH}, 0.08f, 8, Fade(YELLOW, 0.4f));

  int ptSz = (int)(pW * 0.07f);
  const char *pt = "PRESETS";
  DrawText(pt, pX + pW / 2 - MeasureText(pt, ptSz) / 2, pY + 8, ptSz, YELLOW);
  DrawLineEx({pX + 10, pY + 35}, {pX + pW - 10, pY + 35}, 1,
             Fade(YELLOW, 0.4f));

  Vector2 mouse = GetMousePosition();

  if (presetCount == 0) {
    int emptyTxtSz = (int)(pW * 0.055f);
    int emptyTxtSz2 = (int)(pW * 0.045f);

    DrawText("No presets saved.",
             pX + pW / 2 - MeasureText("No presets saved.", emptyTxtSz) / 2,
             pY + 50, emptyTxtSz, Fade(WHITE, 0.6f));
    DrawText("Press S to save current.",
             pX + pW / 2 -
                 MeasureText("Press S to save current.", emptyTxtSz2) / 2,
             pY + 50 + emptyTxtSz + 8, emptyTxtSz2, Fade(WHITE, 0.4f));
  } else {
    float rowH = 36.0f;
    for (int i = 0; i < presetCount; i++) {
      float ry = pY + 45 + i * rowH;

      // Three hit zones per row: the name area loads, the X deletes.
      // They don't overlap so a misclick on the name never deletes.
      Rectangle rowRect = {pX + 6, ry, pW - 40, rowH - 3};
      Rectangle delRect = {pX + pW - 32, ry + 4, 26, rowH - 10};

      bool hovered = CheckCollisionPointRec(mouse, rowRect);
      bool delHov = CheckCollisionPointRec(mouse, delRect);

      if (hovered)
        DrawRectangleRounded(rowRect, 0.15f, 4, Fade(YELLOW, 0.1f));

      // Truncate long names so they don't overflow into the buttons
      char truncName[22];
      snprintf(truncName, sizeof(truncName), "%d. %.16s", i + 1,
               presets[i].name);
      DrawText(truncName, pX + 10, ry + 9, 15,
               hovered ? YELLOW : Fade(WHITE, 0.85f));

      // Delete button — red X, clearly destructive
      DrawRectangleRounded(delRect, 0.2f, 4,
                           delHov ? Fade(RED, 0.4f) : Fade(RED, 0.08f));
      DrawRectangleRoundedLines(delRect, 0.2f, 4,
                                delHov ? RED : Fade(RED, 0.3f));
      DrawText("X", delRect.x + 8, delRect.y + 3, 14,
               delHov ? RED : Fade(RED, 0.6f));

      // Click name row to load preset into customSettings
      if (hovered && !delHov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        customSettings = presets[i].settings;

      // Click X to delete — shift remaining presets down one slot
      if (delHov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int j = i; j < presetCount - 1; j++)
          presets[j] = presets[j + 1];
        presetCount--;
        savePresets();
        break; // iterator is now stale, exit loop
      }
    }
  }

  // SAVE PRESET button anchored to bottom of panel
  Rectangle saveRect = {pX + 8, pY + pH - 38, pW - 16, 28};
  bool saveHov = CheckCollisionPointRec(mouse, saveRect);
  DrawRectangleRounded(saveRect, 0.2f, 4,
                       saveHov ? Fade(GREEN, 0.25f) : Fade(GREEN, 0.06f));
  DrawRectangleRoundedLines(saveRect, 0.2f, 4,
                            saveHov ? GREEN : Fade(GREEN, 0.35f));
  int saveTxtSz = (int)(pW * 0.05f);
  DrawText("SAVE PRESET",
           pX + pW / 2 - MeasureText("SAVE PRESET", saveTxtSz) / 2,
           pY + pH - 30, saveTxtSz, saveHov ? GREEN : Fade(GREEN, 0.6f));

  if (saveHov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
      presetCount < MAX_PRESETS) {
    namingPreset = true;
    presetNameLen = 0;
    memset(presetNameBuf, 0, sizeof(presetNameBuf));
  }
}

// ─── handleCustomMenuInput ───────────────────────────────────
//
//  Input is split into three phases:
//    1. Preset naming mode — captures all keyboard, blocks everything else.
//    2. Mouse — processed unconditionally after naming check.
//    3. Keyboard — standard nav + value editing.
//
//  The clampF lambda is defined once and reused by both mouse
//  scroll and keyboard left/right, keeping the range logic in
//  one place and avoiding drift if limits ever change.
//
void Game::handleCustomMenuInput() {
  int W = GetScreenWidth();
  int H = GetScreenHeight();
  Vector2 mouse = GetMousePosition();

  // Card geometry — must match drawCustomMenu exactly so mouse
  // hit zones line up with what the player sees.
  float cardW = W * 0.58f;
  float cardX = W * 0.04f;
  float cardH = H * 0.072f;
  float startY = H * 0.14f;

  // ── Phase 1: preset naming ─────────────────────────────────
  if (namingPreset) {
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 32 && key <= 126 && presetNameLen < 31) {
        presetNameBuf[presetNameLen++] = (char)key;
        presetNameBuf[presetNameLen] = '\0';
      }
      key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && presetNameLen > 0)
      presetNameBuf[--presetNameLen] = '\0';

    if (IsKeyPressed(KEY_ENTER) && presetNameLen > 0 &&
        presetCount < MAX_PRESETS) {
      CustomPreset &p = presets[presetCount++];
      strncpy(p.name, presetNameBuf, 31);
      p.name[31] = '\0';
      p.settings = customSettings;
      savePresets();
      namingPreset = false;
      memset(presetNameBuf, 0, sizeof(presetNameBuf));
      presetNameLen = 0;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      namingPreset = false;
      memset(presetNameBuf, 0, sizeof(presetNameBuf));
      presetNameLen = 0;
    }
    return; // block all other input while naming
  }

  // ── Shared clamp helper ────────────────────────────────────
  auto clampF = [](float v, float mn, float mx) {
    return v < mn ? mn : (v > mx ? mx : v);
  };

  // ── Phase 2: mouse ─────────────────────────────────────────

  // Clicking the page indicator text toggles pages
  Rectangle pageRect = {float(W) * 0.04f, float(H) * 0.085f, 260, 20};
  if (CheckCollisionPointRec(mouse, pageRect) &&
      IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    customPage = 1 - customPage;
    customSelected = 0;
  }

  // Each card: left-click selects it, scroll wheel changes its value
  int paramCount2 = (customPage == 0) ? 9 : 8;
  for (int i = 0; i < paramCount2; i++) {
    float y = startY + i * (cardH + 6);
    Rectangle cardRect = {cardX, y, cardW, cardH};
    if (!CheckCollisionPointRec(mouse, cardRect))
      continue;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      customSelected = i;

    float wheel = GetMouseWheelMove();
    if (wheel == 0)
      continue;

    customSelected = i;
    float delta = (wheel > 0) ? 1.0f : -1.0f;

    if (customPage == 0) {
      switch (i) {
      case 0:
        customSettings.rows = clampF(customSettings.rows + delta, 1, 15);
        break;
      case 1:
        customSettings.cols = clampF(customSettings.cols + delta, 1, 20);
        break;
      case 2:
        customSettings.lives = clampF(customSettings.lives + delta, 1, 20);
        break;
      case 3:
        customSettings.alienSpeed =
            clampF(customSettings.alienSpeed + delta * 0.05f, 0.05f, 2.0f);
        break;
      case 4:
        customSettings.fireRateScale =
            clampF(customSettings.fireRateScale + delta * 0.1f, 0.1f, 5.0f);
        break;
      case 5:
        customSettings.laserSpeed =
            -(clampF(-customSettings.laserSpeed + delta, 5, 50));
        break;
      case 6:
        customSettings.laserDamage =
            clampF(customSettings.laserDamage + delta, 1, 10);
        break;
      case 7:
        customSettings.alienHP = clampF(customSettings.alienHP + delta, 1, 5);
        break;
      case 8:
        customSettings.stepSize =
            clampF(customSettings.stepSize + delta, 5, 40);
        break;
      }
    } else {
      switch (i) {
      case 0:
        customSettings.descentSpeed =
            clampF(customSettings.descentSpeed + delta, 5, 40);
        break;
      case 1:
        customSettings.gridStartY =
            clampF(customSettings.gridStartY + delta * 0.01f, 0.05f, 0.50f);
        break;
      case 2:
        customSettings.alienSpacing =
            clampF(customSettings.alienSpacing + delta, 30, 80);
        break;
      case 3:
        customSettings.startingLevel =
            clampF(customSettings.startingLevel + delta, 1, 20);
        break;
      case 4:
        customSettings.ufoFrequency =
            clampF(customSettings.ufoFrequency + delta, 5, 60);
        break;
      case 5:
        customSettings.levelSpeedScale =
            clampF(customSettings.levelSpeedScale + delta * 0.05f, 0.5f, 1.0f);
        break;
      case 6:
        customSettings.shieldCount =
            clampF(customSettings.shieldCount + delta, 0, 6);
        break;
      case 7:
        customSettings.scoreMultiplier =
            clampF(customSettings.scoreMultiplier + delta * 0.5f, 1.0f, 5.0f);
        break;
      }
    }
  }

  // PLAY / BACK buttons — geometry must match drawCustomMenu
 Rectangle playRect = {cardX + cardW - 120, float(H) - 45, 120, 30};
  if (CheckCollisionPointRec(mouse, playRect) &&
      IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    resetCustom();
    state = PLAYING;
  }

  Rectangle backRect = {cardX, float(H) - 45, 100, 30};
  if (CheckCollisionPointRec(mouse, backRect) &&
      IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    state = DIFFICULTY_SELECT;

  // ── Phase 3: keyboard ──────────────────────────────────────
  int paramCount = (customPage == 0) ? 9 : 8;

  if (IsKeyPressed(KEY_BACKSPACE)) {
    state = DIFFICULTY_SELECT;
    return;
  }
  if (IsKeyPressed(KEY_TAB)) {
    customPage = 1 - customPage;
    customSelected = 0;
    return;
  }
  if (IsKeyPressed(KEY_UP))
    customSelected = (customSelected - 1 + paramCount) % paramCount;
  if (IsKeyPressed(KEY_DOWN))
    customSelected = (customSelected + 1) % paramCount;

  // R randomises every field so players can discover wild configs
  if (IsKeyPressed(KEY_R)) {
    customSettings.rows = GetRandomValue(1, 15);
    customSettings.cols = GetRandomValue(1, 20);
    customSettings.lives = GetRandomValue(1, 20);
    customSettings.alienSpeed = GetRandomValue(5, 200) / 100.0f;
    customSettings.fireRateScale = GetRandomValue(1, 50) / 10.0f;
    customSettings.laserSpeed = -GetRandomValue(5, 50);
    customSettings.laserDamage = GetRandomValue(1, 10);
    customSettings.alienHP = GetRandomValue(1, 5);
    customSettings.stepSize = GetRandomValue(5, 40);
    customSettings.descentSpeed = GetRandomValue(5, 40);
    customSettings.gridStartY = GetRandomValue(5, 50) / 100.0f;
    customSettings.alienSpacing = float(GetRandomValue(30, 80));
    customSettings.startingLevel = GetRandomValue(1, 20);
    customSettings.ufoFrequency = float(GetRandomValue(5, 60));
    customSettings.levelSpeedScale = GetRandomValue(50, 100) / 100.0f;
    customSettings.shieldCount = GetRandomValue(0, 6);
    customSettings.scoreMultiplier = GetRandomValue(2, 10) / 2.0f;
    return;
  }

  if (IsKeyPressed(KEY_S) && presetCount < MAX_PRESETS) {
    namingPreset = true;
    presetNameLen = 0;
    memset(presetNameBuf, 0, sizeof(presetNameBuf));
    return;
  }

  // F1-F10 quick-load presets by index
  for (int i = 0; i < presetCount && i < MAX_PRESETS; i++) {
    if (IsKeyPressed(KEY_F1 + i)) {
      customSettings = presets[i].settings;
      return;
    }
  }

  // LEFT/RIGHT change the currently selected parameter.
  // IsKeyPressedRepeat fires repeatedly while held for fast scrubbing.
  float delta = 0;
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT))
    delta = -1;
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT))
    delta = 1;

  if (delta == 0) {
    if (IsKeyPressed(KEY_ENTER)) {
      resetCustom();
      state = PLAYING;
    }
    return;
  }

  if (customPage == 0) {
    switch (customSelected) {
    case 0:
      customSettings.rows = clampF(customSettings.rows + delta, 1, 15);
      break;
    case 1:
      customSettings.cols = clampF(customSettings.cols + delta, 1, 20);
      break;
    case 2:
      customSettings.lives = clampF(customSettings.lives + delta, 1, 20);
      break;
    case 3:
      customSettings.alienSpeed =
          clampF(customSettings.alienSpeed + delta * 0.05f, 0.05f, 2.0f);
      break;
    case 4:
      customSettings.fireRateScale =
          clampF(customSettings.fireRateScale + delta * 0.1f, 0.1f, 5.0f);
      break;
    case 5:
      customSettings.laserSpeed =
          clampF(-customSettings.laserSpeed + delta, 5, 50);
      customSettings.laserSpeed = -customSettings.laserSpeed;
      break;
    case 6:
      customSettings.laserDamage =
          clampF(customSettings.laserDamage + delta, 1, 10);
      break;
    case 7:
      customSettings.alienHP = clampF(customSettings.alienHP + delta, 1, 5);
      break;
    case 8:
      customSettings.stepSize = clampF(customSettings.stepSize + delta, 5, 40);
      break;
    }
  } else {
    switch (customSelected) {
    case 0:
      customSettings.descentSpeed =
          clampF(customSettings.descentSpeed + delta, 5, 40);
      break;
    case 1:
      customSettings.gridStartY =
          clampF(customSettings.gridStartY + delta * 0.01f, 0.05f, 0.50f);
      break;
    case 2:
      customSettings.alienSpacing =
          clampF(customSettings.alienSpacing + delta, 30, 80);
      break;
    case 3:
      customSettings.startingLevel =
          clampF(customSettings.startingLevel + delta, 1, 20);
      break;
    case 4:
      customSettings.ufoFrequency =
          clampF(customSettings.ufoFrequency + delta, 5, 60);
      break;
    case 5:
      customSettings.levelSpeedScale =
          clampF(customSettings.levelSpeedScale + delta * 0.05f, 0.5f, 1.0f);
      break;
    case 6:
      customSettings.shieldCount =
          clampF(customSettings.shieldCount + delta, 0, 6);
      break;
    case 7:
      customSettings.scoreMultiplier =
          clampF(customSettings.scoreMultiplier + delta * 0.5f, 1.0f, 5.0f);
      break;
    }
  }
}

void Game::drawPauseMenu() {
  int W = GetScreenWidth();
  int H = GetScreenHeight();

  // Draw the game behind it (already drawn before this is called)
  // Dark overlay
  DrawRectangle(0, 0, W, H, Fade(BLACK, 0.65f));

  // Panel
  float panelW = W * 0.35f;
  float panelH = H * 0.40f;
  float panelX = (W - panelW) / 2.0f;
  float panelY = (H - panelH) / 2.0f;

  DrawRectangleRounded({panelX, panelY, panelW, panelH}, 0.1f, 8,
                       {20, 20, 20, 240});
  DrawRectangleRoundedLines({panelX, panelY, panelW, panelH}, 0.1f, 8, YELLOW);

  // Title
  const char *title = "PAUSED";
  int titleSize = W / 18;
  DrawText(title, W / 2 - MeasureText(title, titleSize) / 2,
           panelY + panelH * 0.08f, titleSize, YELLOW);

  DrawLineEx({panelX + 20, panelY + panelH * 0.30f},
             {panelX + panelW - 20, panelY + panelH * 0.30f}, 2, YELLOW);

  // Options
  const char *opts[3] = {"RESUME", "MAIN MENU", "QUIT GAME"};
  Color colors[3] = {GREEN, YELLOW, RED};

  for (int i = 0; i < 3; i++) {
    float optY = panelY + panelH * (0.38f + i * 0.22f);
    bool sel = (pauseSelected == i);
    int fsize = W / 30;

    if (sel)
      DrawText(">", panelX + 18, optY, fsize, colors[i]);

    DrawText(opts[i], W / 2 - MeasureText(opts[i], fsize) / 2, optY, fsize,
             sel ? colors[i] : Fade(colors[i], 0.45f));
  }

  // Hint
  const char *hint = "UP/DOWN: Navigate    ENTER: Confirm";
  int hintSize = W / 65;
  DrawText(hint, W / 2 - MeasureText(hint, hintSize) / 2, panelY + panelH - 28,
           hintSize, DARKGRAY);
}
void Game::drawScrollingBackground() {
  updateMenuStars();
  drawMenuStars();
}
