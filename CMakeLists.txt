cmake_minimum_required(VERSION 3.16)
project(topdown)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Collect all source files for the top-down game.
# Each SDL_App* callback lives in its own file under topdown/ for clarity.
add_executable(topdown
  topdown/init.c
  topdown/events.c
  topdown/update.c
  topdown/iterate.c
  topdown/render.c
  topdown/quit.c
  topdown/camera.c
  topdown/entity/player.c
  topdown/entity/enemy.c
)

# Make the topdown/ directory available for #include "common.h" etc.
target_include_directories(topdown PRIVATE topdown)

# Link to the actual SDL3 library.
target_link_libraries(topdown PRIVATE SDL3::SDL3)
