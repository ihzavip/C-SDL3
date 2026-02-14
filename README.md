# C-SDL3

<!--toc:start-->

- [C-SDL3](#c-sdl3)
<!--toc:end-->

Build game with C and SDL3

run main
gcc src/main.c src/helper.c game/main.c -Iinclude -o build/main.exe

Build SDL on Unix
cmake --build ./build

Build SDL on windows
cmake -S . -B build -G "MinGW Makefiles"

To run
./build/hello
