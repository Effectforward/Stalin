cmake_minimum_required(VERSION 3.31.10)
project(Stalin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# This stores all fetched content in a central location outside the build folder
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/external")


include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
    GIT_SHALLOW    ON
)

FetchContent_MakeAvailable(raylib)

add_executable(stalin 
    src/main.cpp
    include/spaceship.hpp
    src/spaceship.cpp
    include/game.hpp
    src/game.cpp
    include/laser.hpp
    src/laser.cpp
)

target_include_directories(stalin PRIVATE ${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

target_link_libraries(stalin PRIVATE raylib Threads::Threads)

if(MSVC)
    target_compile_options(stalin PRIVATE /W4 /WX)
else()
    target_compile_options(stalin PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

add_custom_command(TARGET stalin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/bin/assets
    COMMENT "Copying assets directory to build output"
)


add_custom_command(TARGET stalin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Syncing compile_commands.json for clangd"
)
