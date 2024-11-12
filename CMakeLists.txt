cmake_minimum_required(VERSION 3.15)
project(MyCProject C)

set(CMAKE_C_STANDARD 99)

add_executable(MyCExecutable main.c
    Tetris/Tetris.c
    )