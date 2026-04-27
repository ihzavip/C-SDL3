@echo off
setlocal

REM -----------------------------------------------------------------------
REM  build-windows.bat — one-shot build script for topdown on Windows
REM
REM  Prerequisites:
REM    - CMake 3.16+  (https://cmake.org/download/)
REM    - Visual Studio 2019/2022 with "Desktop development with C++" workload
REM      (or another generator — see comment below)
REM    - SDL3_image source in vendored\SDL_image  (run step 1 below once)
REM
REM  Step 1 — vendor SDL_image (run once):
REM    git clone --depth=1 --branch release-3.2.4 ^
REM      https://github.com/libsdl-org/SDL_image.git vendored\SDL_image
REM
REM  Step 2 — build (run this script):
REM    build-windows.bat
REM
REM  The binary lands at:  build\Debug\topdown.exe
REM  Run from the repo root so asset paths (media\...) resolve correctly:
REM    build\Debug\topdown.exe
REM -----------------------------------------------------------------------

REM Detect Visual Studio generator automatically via vswhere.
set GENERATOR=
for /f "usebackq tokens=*" %%i in (
  `"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" ^
    -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
    -property installationVersion 2^>nul`
) do (
    set VS_VER=%%i
)

if defined VS_VER (
    REM Extract major version number (e.g. "17" from "17.x.x")
    for /f "tokens=1 delims=." %%v in ("%VS_VER%") do set VS_MAJOR=%%v
    if "%VS_MAJOR%"=="17" set GENERATOR=Visual Studio 17 2022
    if "%VS_MAJOR%"=="16" set GENERATOR=Visual Studio 16 2019
)

if not defined GENERATOR (
    REM Try Ninja, then MinGW Makefiles.
    where ninja >nul 2>&1 && set GENERATOR=Ninja
)
if not defined GENERATOR (
    where mingw32-make >nul 2>&1 && set GENERATOR=MinGW Makefiles
)
if not defined GENERATOR (
    echo Could not detect Visual Studio, Ninja, or MinGW.  Please install one.
    exit /b 1
)

echo Using generator: %GENERATOR%
echo.

REM Check that SDL_image is vendored.
if not exist "vendored\SDL_image\CMakeLists.txt" (
    echo ERROR: vendored\SDL_image not found.
    echo.
    echo Run this once to vendor SDL_image:
    echo   git clone --depth=1 --branch release-3.2.4 ^
    echo     https://github.com/libsdl-org/SDL_image.git vendored\SDL_image
    echo.
    exit /b 1
)

REM Wipe cache to avoid generator mismatch errors on reconfigure.
if exist "build\CMakeCache.txt" (
    echo Clearing stale CMake cache...
    del /q "build\CMakeCache.txt"
    rmdir /s /q "build\CMakeFiles"
)

REM Configure.
cmake -S . -B build -G "%GENERATOR%" -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 (
    echo.
    echo CMake configure failed.
    exit /b 1
)

REM Build.
cmake --build build --target topdown --config Debug
if errorlevel 1 (
    echo.
    echo Build failed.
    exit /b 1
)

echo.
echo Build succeeded.  Run with:
echo   build\Debug\topdown.exe
