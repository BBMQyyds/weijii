cmake_minimum_required(VERSION 2.8.12.2)
project(weiji C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-std=c99")

add_executable(weiji main.c dispose.c dispose.h data.c data.h user.c user.h)

