cmake_minimum_required(VERSION 3.10)
project(puzzle_game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

add_library(glad src/gl.c)
target_include_directories(glad PUBLIC include)

file(GLOB SRC_FILES src/*.cpp src/*.c)
add_executable(game ${SRC_FILES})

target_link_libraries(game glad glfw3 opengl32)