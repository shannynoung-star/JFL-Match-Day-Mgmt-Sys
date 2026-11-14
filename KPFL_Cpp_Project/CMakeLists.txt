# For CLion / Visual Studio / VS Code CMake users.
cmake_minimum_required(VERSION 3.10)
project(KPFL CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(kpfl src/main.cpp)
add_executable(kpfl_tests src/tests.cpp)
