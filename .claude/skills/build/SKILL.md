---
name: build
description: Build and run the topdown SDL3 game
---

Build the game:
```
cmake --build build --target topdown -j$(nproc)
```

Run from the project root:
```
./build/Debug/topdown
```

If CMake needs to be reconfigured first (e.g. after changing CMakeLists.txt):
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```
