# Raylib C++ Template

A cross-platform C++ template using CMake and raylib. Configured for Windows, Linux, and macOS.

## Usage

Click the green **"Use this template"** button on GitHub to create your own project from this template. This generates a fresh repository with clean history.

## Features

- Cross-platform build system using CMake
- raylib automatically downloaded and built via FetchContent
- Code formatting with clang-format
- Pre-configured VSCode environment
- C++17 standard

## Prerequisites

Install CMake (3.28+), Git, and platform-specific build tools:

### Ubuntu / Debian
```bash
sudo apt install cmake git build-essential libx11-dev libxrandr-dev libxinerama-dev libxi-dev libxext-dev libxcursor-dev mesa-common-dev
```

### Fedora / RHEL / CentOS
```bash
sudo dnf install cmake git gcc-c++ libX11-devel libXrandr-devel libXinerama-devel libXi-devel libXext-devel libXcursor-devel mesa-libGL-devel
```

### Arch / Manjaro
```bash
sudo pacman -S cmake git base-devel libx11 libxrandr libxinerama libxi libxext libxcursor mesa
```

### openSUSE
```bash
sudo zypper install cmake git gcc-c++ libX11-devel libXrandr-devel libXinerama-devel libXi-devel libXext-devel libXcursor-devel Mesa-devel
```

### Alpine
```bash
apk add cmake git musl-dev gcc g++ libx11-dev libxrandr-dev libxinerama-dev libxi-dev libxext-dev libxcursor-dev mesa-dev
```

### macOS
```bash
brew install cmake git
```

### Windows
1. Download CMake: https://cmake.org/download/
- During installation, check "Add CMake to system PATH"
2. Download Git: https://git-scm.com/download/win
3. Download Visual Studio Build Tools: https://visualstudio.microsoft.com/visual-cpp-build-tools/
- During installation, select "Desktop development with C++"

## VS Code Workflow (Recommended)

This template is fully pre-configured for a zero-friction experience in **Visual Studio Code**:

1. Open the project folder in VS Code.
2. VS Code will show a popup recommending extensions (`clangd`, `CodeLLDB`, etc.). Click **Install**.
3. Press **F5**.

The project will automatically configure CMake, build the game, and attach a cross-platform debugger. It works perfectly out-of-the-box on Windows, Linux, and macOS without manually typing any terminal commands.

## Manual Terminal Building (Alternative)

If you prefer building from the terminal:

> **Windows users:** If using MSVC, run these commands from the "Developer Command Prompt for VS".

```bash
git clone https://github.com/Effectforward/raylib-cpp-template.git
cd raylib-cpp-template
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The first build takes slightly longer as raylib is downloaded and compiled. Subsequent builds are nearly instantaneous.

On successful build, you can run the executable:

```bash
./build/bin/game              # Linux/macOS
./build/bin/game.exe          # Windows
```

## Development

### Adding Source Files

Edit `CMakeLists.txt` and add new files to the `add_executable` list:

```cmake
add_executable(game 
    src/main.cpp
    src/new_file.cpp
)
```

If using VS Code, just press **F5** to automatically re-configure and rebuild. If using the terminal, run `cmake --build build`.

### Code Style

Code formatting is configured with `clang-format`:
- Indentation: tabs (displayed as 4 spaces)
- Line length: 100 characters
- Brace style: K&R

Apply formatting manually: `clang-format -i src/filename.cpp`
**Note:** The provided VS Code environment automatically formats your code on save.

## Project Structure

```
.
├── CMakeLists.txt           # Build configuration
├── .clang-format            # Code formatting rules
├── .gitignore               # Git ignore patterns
├── README.md                # Setup guide
├── .vscode/                 # VSCode configuration
├── src/
│   └── main.cpp             # Main source file
└── include/
    └── game.h               # Header file
```
