cmake_minimum_required(VERSION 3.15)
project(advent_of_code_2019)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(advent_of_code_2019
        src/day1.cpp
        src/main.cpp)
