cmake_minimum_required(VERSION 3.10)
project(utf8c C)

set(CMAKE_C_STANDARD 99)
add_library(utf8c utf8c/utf8c.c)
target_include_directories(utf8c PUBLIC utf8c)
add_executable(main main.c)
target_link_libraries(main PRIVATE utf8c)
