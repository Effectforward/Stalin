
#include "aliens.hpp"
#include "laser.hpp"
#include "obstacles.hpp"
#include "raylib.h"

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
  if (maxY > GetScreenHeight() * 0.7f) {
    spaceship.setLaserSpeed(-25); // Faster lasers
    spaceship.setLaserDamage(3);  // Triple damage
  }

  // If fewer than 10 aliens remain, go to hyper-speed
  if (aliens.size() < 25 && !aliens.empty()) {
    spaceship.setLaserSpeed(-40); // Hyper lasers
    spaceship.setLaserDamage(10); // Ultimate damage
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
  }

  // Fixed spacing that looks tight and arcade-like
  float spacingX = 55.0f;
  float spacingY = 45.0f;

  // Total grid pixel dimensions
  float gridW = (cols - 1) * spacingX;
  float gridH = (rows - 1) * spacingY;

  // Centre horizontally, start 15% from top
  float startX = (GetScreenWidth() - gridW) / 2.0f;
  float startY = (GetScreenHeight() * 0.15f);

  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      int alienType;
      if (row == 0)
        alienType = 3;
      else if (row == 1 || row == 2)
        alienType = 2;
      else
        alienType = 1;

      float x = startX + col * spacingX;
      float y = startY + row * spacingY;
      aliens.push_back(Alien(alienType, {x, y}));
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
    }

    // Linear interpolation based on remaining aliens
    // We assume a full grid size for each difficulty to calculate the ratio
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

    // Play movement sound
    PlaySound(moveSounds[currentMoveSound]);
    currentMoveSound = (currentMoveSound + 1) % 4;

    // Move all aliens horizontally by a "step" (e.g., 15 pixels)
    int stepSize = 15;
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
      moveAliensDown(15);   // Move the block down by 15 pixels

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
  const int obstacleCount = 4;
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

    alienShootInterval = (GetRandomValue(1, 6) / 10.0) * scale;

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
            score += 30;
          else if (alienIt->type == 2)
            score += 20;
          else
            score += 10;
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
  alienMoveInterval *= 0.9;
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
      s.size  = 1.0f;
    } else if (i < 70) {
      s.speed = GetRandomValue(80, 120) / 100.0f;
      s.size  = 1.5f;
    } else {
      s.speed = GetRandomValue(180, 220) / 100.0f;
      s.size  = 2.0f;
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
    if (s.x < 0)                s.x = GetScreenWidth();
    if (s.x > GetScreenWidth()) s.x = 0;
  }
}
void Game::drawMenuStars() {
  for (int i = 0; i < 80; i++) {
    auto &s = menuStars[i];
    Color c;
    if (i < 50)       c = Fade(WHITE, 0.4f);
    else if (i < 70)  c = Fade(WHITE, 0.75f);
    else              c = WHITE;
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

void Game::handleMainMenuInput() {
    if (IsKeyPressed(KEY_Q)) {
   shouldQuit = true;
    return;
  }
  
  // ENTER to proceed to difficulty select
  if (IsKeyPressed(KEY_ENTER)) {
    selectedDifficulty = 2; // Reset to MEDIUM
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

  DiffOption opts[7] = {
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
  };
  // Card layout
  float cardH = H * 0.085f;
  float cardW = W * 0.65f;
  float cardX = (W - cardW) / 2.0f;
  float totalH = 7 * cardH + 6 * 8;
  float startY = H * 0.22f; // starts just below the separator line
  for (int i = 0; i < 7; i++) {
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

void Game::handleDifficultyInput() {
  if (IsKeyPressed(KEY_UP))
    selectedDifficulty = (selectedDifficulty - 1 + 7) % 7;

  if (IsKeyPressed(KEY_DOWN))
    selectedDifficulty = (selectedDifficulty + 1) % 7;

  if (IsKeyPressed(KEY_BACKSPACE))
    state = MENU;

  if (IsKeyPressed(KEY_ENTER)) {
    difficulty = (Difficulty)selectedDifficulty;
    Reset();
    state = PLAYING;
  }

  // Keep the old number keys working too
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


