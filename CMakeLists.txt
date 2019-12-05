cmake_minimum_required(VERSION 3.15)
project(advent_of_code_2019)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)

add_executable(advent_of_code_2019
        src/day1.cpp
        src/day2.cpp
        src/day3.cpp
        src/day4.cpp
        src/day5.cpp
        src/main.cpp)
