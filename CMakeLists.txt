cmake_minimum_required(VERSION 3.10.0)
project(Rain VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Rain
src/main.cpp
src/lexer/lexer.cpp
src/file/posinfo.cpp
)

include_directories(./src/)