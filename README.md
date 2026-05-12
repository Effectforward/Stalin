# Stalin Invaders

A high-performance, aesthetically pleasing Space Invaders clone built with C++ and Raylib. This project is designed as an OOP demonstration for the BS IT curriculum at the University of Gujrat.

## Features

- **Dynamic Difficulty**: 5 distinct difficulty levels (Novice, Easy, Medium, Hard, Stalin) with varying alien counts, speeds, and player lives.
- **Classic Arcade Audio**: Sequential marching sounds, explosion effects, and UFO sirens.
- **UFO Bonus**: Rare UFO appearances for high-score bonuses.
- **Volume Progression**: Endless level progression with scaling difficulty.
- **Premium Aesthetics**: Refined sprites with transparent backgrounds and a modern HUD.
- **OOP Architecture**: Demonstrates core OOP pillars including Inheritance, Polymorphism, Encapsulation, and Abstraction.

## Prerequisites

### Linux (Fedora/RHEL)
```bash
sudo dnf install cmake git gcc-c++ libX11-devel libXrandr-devel libXinerama-devel libXi-devel libXext-devel libXcursor-devel mesa-libGL-devel
```

### Linux (Ubuntu/Debian)
```bash
sudo apt install cmake git build-essential libx11-dev libxrandr-dev libxinerama-dev libxi-dev libxext-dev libxcursor-dev mesa-common-dev
```

## Build and Run

1. **Configure and Build**:
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```

2. **Run the Game**:
   ```bash
   ./build/bin/stalin
   ```

## Controls

- **[Left/Right Arrow]**: Move Spaceship
- **[Space]**: Fire Laser
- **[1-5]**: Select Difficulty (Start Menu)
- **[Space]**: Reset/Return to Menu (Game Over)

## Project Structure

- `src/`: Core logic implementation.
- `include/`: Header files and class definitions.
- `assets/`: Game sprites and audio files.
- `docs/`: Project documentation and guidelines.

## Academic OOP Implementation

This project implements the following OOP concepts:
- **Inheritance**: `Spaceship` and `Alien` inherit from a base `Entity` class.
- **Polymorphism**: Virtual `Draw()` and `getRect()` methods in the `Entity` base class.
- **Encapsulation**: Private data members with public interfaces.
- **Abstraction**: Complex raylib interactions abstracted through the `Game` and `Entity` classes.

---
© 2026 Stalin Invaders Team - University of Gujrat
