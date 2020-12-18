cmake_minimum_required(VERSION 3.14)
project(testCodeur C)

set(CMAKE_C_STANDARD 11)

add_executable(codeur src/main.c src/lzw.c src/utils.c)

add_executable(testCodeur src/test_main.c src/lzw.c src/utils.c)