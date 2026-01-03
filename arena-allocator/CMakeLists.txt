cmake_minimum_required(VERSION 3.15)

project(Allocator VERSION 0.5 LANGUAGES C)


add_executable(Allocator
    src/arena.c
    tests/speed.c
)


target_include_directories(Allocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


target_compile_options(Allocator PRIVATE -std=gnu11 -g -O3 -Wall -Wextra -Wpedantic)
