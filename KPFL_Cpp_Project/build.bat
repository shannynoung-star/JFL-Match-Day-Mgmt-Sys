@echo off
REM Windows build using MinGW-w64 / MSYS2 g++ (must be on PATH).
cd /d "%~dp0"
g++ -std=c++17 -Wall -Wextra -O2 -o kpfl.exe src\main.cpp
if errorlevel 1 goto end
g++ -std=c++17 -Wall -Wextra -O2 -o kpfl_tests.exe src\tests.cpp
if errorlevel 1 goto end
kpfl_tests.exe
kpfl.exe
:end
pause
