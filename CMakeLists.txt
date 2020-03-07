cmake_minimum_required(VERSION 3.10)
project(utf8c C)

set(CMAKE_C_STANDARD 99)

add_executable(utf8c main.c utf8c/utf8c.c)
target_include_directories(utf8c PRIVATE utf8c)
