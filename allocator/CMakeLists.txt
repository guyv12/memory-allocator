cmake_minimum_required(VERSION 3.15)

project(Allocator VERSION 0.5 LANGUAGES C)


add_executable(Allocator
    src/arena.c
    tests/general_use.c
)


target_include_directories(Allocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


target_compile_options(Allocator PRIVATE -g -O0 -Wall -Wextra -Wpedantic)
