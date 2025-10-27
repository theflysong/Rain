cmake_minimum_required(VERSION 3.10.0)
project(Rain VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)

add_executable(Rain
src/main.cpp

src/lexer/lexer.cpp
src/lexer/token_type.cpp

src/file/posinfo.cpp
src/file/helper.cpp

src/util/mem/bytebuffer.cpp
)

include_directories(./src/)