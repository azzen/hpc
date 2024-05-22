cmake_minimum_required(VERSION 3.0)
project(HPC_Lab05_power_consumption)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wall -fno-inline -march=native -fno-tree-vectorize")

set(SUM_SRC
    test_power_consumption/src/sum.c
)

add_executable(sum ${SUM_SRC})

set(SEGMENTATION_SIMD_SRC
    segmentation_simd/src/main_segmentation.c
    segmentation_simd/src/k-means.c
    segmentation_simd/src/image.c
)

add_executable(segmentation_simd ${SEGMENTATION_SIMD_SRC})

target_include_directories(segmentation_simd PUBLIC segmentation_simd/include)
target_include_directories(segmentation_simd PUBLIC lib/stb/)
target_link_libraries(segmentation_simd m)

set(SEGMENTATION_SRC
    segmentation/src/main_segmentation.c
    segmentation/src/k-means.c
    segmentation/src/image.c
)

add_executable(segmentation ${SEGMENTATION_SRC})

target_include_directories(segmentation PUBLIC segmentation/include/)
target_include_directories(segmentation PUBLIC lib/stb/)
target_link_libraries(segmentation m)


