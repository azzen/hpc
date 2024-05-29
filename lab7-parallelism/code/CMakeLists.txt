cmake_minimum_required(VERSION 3.10)
project(FindSequence)

set(CMAKE_C_STANDARD 99)

add_executable(find_sequence src/main.c)
add_executable(test src/main2.c)

target_link_libraries(test m)