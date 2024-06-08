cmake_minimum_required(VERSION 3.16)
project(FindSequence)

set(CMAKE_C_STANDARD 11)

add_executable(find_sequence src/main.c)

target_link_libraries(find_sequence m)
target_link_libraries(find_sequence pthread)