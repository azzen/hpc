cmake_minimum_required(VERSION 3.16)
project(FindSequence)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3 -Wall -Wextra -g -gdwarf-3")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
add_executable(find_sequence src/main.c)

target_link_libraries(find_sequence m dl Threads::Threads OpenMP::OpenMP_C)
target_include_directories(find_sequence PRIVATE "lib/coz/include")