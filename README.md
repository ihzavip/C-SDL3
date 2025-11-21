# C-SDL3

<!--toc:start-->

- [C-SDL3](#c-sdl3)
<!--toc:end-->

Build game with C and SDL3

Build SDL on Unix
cmake --build ./build

Build SDL on windows (Please, reconsider your life choices)
cmake -S . -B build -G "MinGW Makefiles"

To run
./build/hello
