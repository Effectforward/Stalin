
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
  if (!run)
    return; // Stop updates if game is over

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
}

void Game::Draw() {
  spaceship.Draw();
  for (auto &laser : spaceship.lasers) {
    laser.Draw();
  }

  for (auto &alien : aliens) {
    alien.Draw();
  }

  for (auto &laser : alienLaser) {
    laser.Draw();
  }

  for (auto &obstacle : obstacles) {
    obstacle.Draw();
  }
  drawUfo();

  // Draw HUD Line
  DrawLineEx({10, float(GetScreenHeight() - 50)},
             {float(GetScreenWidth() - 10), float(GetScreenHeight() - 50)}, 3,
             YELLOW);

  DrawText("LIVES:", 10, GetScreenHeight() - 35, 20, YELLOW);
  float startX = 85.0f;
  for (int i = 0; i < lives; i++) {
    DrawRectangle(startX + i * 30, GetScreenHeight() - 30, 20, 10, YELLOW);
  }

  // Draw Level
  DrawText(TextFormat("LEVEL %02d", level), GetScreenWidth() - 120,
           GetScreenHeight() - 35, 20, YELLOW);

  // Draw Scores
  DrawText("SCORE", 50, 20, 20, YELLOW);
  DrawText(TextFormat("%05d", score), 50, 45, 20, YELLOW);

  DrawText("HI-SCORE", GetScreenWidth() - 150, 20, 20, YELLOW);
  DrawText(TextFormat("%05d", highScore), GetScreenWidth() - 150, 45, 20,
           YELLOW);

  // Draw Border (Encompassing the whole game area except HUD)
  DrawRectangleLinesEx(
      {5, 5, float(GetScreenWidth() - 10), float(GetScreenHeight() - 55)}, 3,
      YELLOW);

  if (!run) {
    DrawText("GAME OVER", GetScreenWidth() / 2 - 100,
             GetScreenHeight() / 2 - 50, 40, RED);
    DrawText("Press SPACE to Play Again", GetScreenWidth() / 2 - 130,
             GetScreenHeight() / 2 + 10, 20, WHITE);
  }
}

void Game::handleInput() {
  if (!run) {
    if (IsKeyDown(KEY_SPACE)) {
      Reset();
    }
    return;
  }

  if (IsKeyDown(KEY_LEFT)) {
    spaceship.moveLeft();
  } else if (IsKeyDown(KEY_RIGHT)) {
    spaceship.moveRight();
  } else if (IsKeyDown(KEY_SPACE)) {
    spaceship.Firelaser();
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
  // OLD CODE:
  // for (int row = 0; row < 7; row++) {
  // 	for (int column = 0; column < 10; column++) {
  // NEW CODE: Increase rows to 10 and columns to 15
  for (int row = 0; row < 10; row++) {
    for (int column = 0; column < 15; column++) {
      int alienType;
      if (row == 0) {
        alienType = 3;
      } else if (row == 1 || row == 2) {
        alienType = 2;

      } else {
        alienType = 1;
      }

      // OLD CODE:
      // float x = 75 + column * 55;
      // float y = 110 + row * 55;
      // NEW CODE: Decrease spacing from 55 to 35
      float x = 75 + column * 35;
      float y = 110 + row * 35;
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
    // As aliens are destroyed, size decreases, so the interval becomes smaller
    // (faster!). E.g., 150 aliens * 0.003 = 0.45s. 1 alien * 0.003 = 0.003s. We
    // add a base 0.05 so they don't go infinitely fast.
    alienMoveInterval = (aliens.size() * 0.003) + 0.05;

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
    // This makes the alien fire rate unpredictable like the arcade game, change
    // this to increase or decrease the fire rate
    alienShootInterval = GetRandomValue(1, 6) / 10.0;

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
        // Instead of deleting immediately, we reduce the alien's HP
        alienIt->hp -= 1;
        laser.active = false; // The laser is destroyed on impact

        // If HP reaches 0, the alien is defeated and removed from the list
        if (alienIt->hp <= 0) {
          // Add score based on type
          if (alienIt->type == 3)
            score += 30;
          else if (alienIt->type == 2)
            score += 20;
          else
            score += 10;

          // Update High Score
          if (score > highScore)
            highScore = score;

          PlaySound(alienDeathSound);
          alienIt = aliens.erase(alienIt);
        } else {
          // If still alive, move to the next alien in the list
          ++alienIt;
        }
        break; // Stop checking this laser since it already hit an alien
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
        score += 100; // Bonus score for UFO
        if (score > highScore)
          highScore = score;
        PlaySound(ufoHighSound);
        StopSound(ufoLowSound);
      }
    }

    // 3. Check collision with Obstacles (Separated from aliens for performance)
    if (laser.active) {
      for (auto &obstacle : obstacles) {
        // Dynamic Damage: The width of the impact depends on the laser's damage
        // stat
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

  // --- ALIEN LASER COLLISIONS ---
  for (auto &laser : alienLaser) {
    // 3. Check collision with the Spaceship (The Player)
    // BUG FIX: Added laser.active check to prevent multiple hits from the same
    // laser
    if (laser.active &&
        CheckCollisionRecs(laser.getRect(), spaceship.getRect())) {
      laser.active = false;
      lives--; // decreases total lives by 1
      PlaySound(explosionSound);
      if (lives <= 0) {
        gameOver();
      }
    }

    // 4. Check collision with Obstacles (Aliens can also damage shields)
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
            // Stronger alien lasers can punch through more blocks
          } else {
            ++blockIt;
          }
        }
        if (!laser.active)
          break;
      }
    }
  }
  // --- ALIEN BODY COLLISIONS (Direct Contact) ---
  for (auto &alien : aliens) {
    // 5. Alien vs Obstacles (Aliens "crush" blocks they touch)
    for (auto &obstacle : obstacles) {
      auto blockIt = obstacle.blocks.begin();
      while (blockIt != obstacle.blocks.end()) {
        if (CheckCollisionRecs(alien.getRect(), blockIt->getRect())) {
          // Alien is strong enough to instantly crush obstacle blocks
          blockIt = obstacle.blocks.erase(blockIt);
        } else {
          ++blockIt;
        }
      }
    }

    // 6. Alien vs Spaceship (Direct contact is fatal)
    if (CheckCollisionRecs(alien.getRect(), spaceship.getRect())) {
      PlaySound(explosionSound);
      gameOver();
    }
  }
}

void Game::gameOver() { run = false; }

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
  run = true;
  alienDirection = 1;
  timeLastAlienMoved = GetTime();
  alienMoveInterval = 0.5;
  timeLastAlienShot = GetTime();
  alienShootInterval = GetRandomValue(10, 30) / 10.0;
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