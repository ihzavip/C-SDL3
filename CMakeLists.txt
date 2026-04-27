cmake_minimum_required(VERSION 3.16)
project(topdown)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# SDL3 — built from source (vendored)
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# SDL3_image — use vendored source if present, otherwise find system install.
# To vendor it: git clone --depth=1 --branch release-3.2.4 \
#   https://github.com/libsdl-org/SDL_image.git vendored/SDL_image
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL_image/CMakeLists.txt")
    set(SDL3IMAGE_VENDORED ON)
    # Point SDL_image at our already-configured SDL3 target so it doesn't
    # try to fetch its own copy.
    set(SDL3_DIR "${CMAKE_BINARY_DIR}/vendored/SDL")
    add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
else()
    set(SDL3IMAGE_VENDORED OFF)
    find_package(SDL3_image REQUIRED)
endif()

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

# Link SDL3 and SDL3_image.
target_link_libraries(topdown PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)

# On Windows, copy SDL3.dll / SDL3_image.dll next to the .exe automatically.
if(WIN32)
    add_custom_command(TARGET topdown POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:topdown>
            $<TARGET_FILE_DIR:topdown>
        COMMAND_EXPAND_LISTS
    )
endif()
