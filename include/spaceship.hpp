#include "entity.hpp"
#include "laser.hpp"
#include "raylib.h"
#include <vector>
// Spaceship class
class Spaceship : public Entity {
public:
  Spaceship();
  ~Spaceship();
  void Draw() override;
void Draw(Color tint);
  void moveLeft();
  void moveRight();
  void Firelaser();
  // we need a rectangle to check collison uisng built in raylib function
  Rectangle getRect() override;
  void Reset();
  std::vector<Laser> lasers; // this will hold all the lasers

  // Power-up methods
  void setLaserSpeed(int s) { laserSpeed = s; }
  void setLaserDamage(int d) { laserDamage = d; }

private:
  int speed = 6;
  int laserSpeed = -15;  // Default speed
  int laserDamage = 1;   // Default damage
  double lastFireTime;
};