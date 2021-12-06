cmake_minimum_required(VERSION 3.21)
project(untitled)

set(CMAKE_CXX_STANDARD 14)

add_executable(untitled smash.cpp Commands.cpp signals.cpp Commands.h signals.h)
