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

set(SEGMENTATION_PERF_SRC
    segmentation_perf/src/main_segmentation.c
    segmentation_perf/src/k-means.c
    segmentation_perf/src/image.c
)

add_executable(segmentation_perf ${SEGMENTATION_PERF_SRC})

target_include_directories(segmentation_perf PUBLIC segmentation_perf/include/)
target_include_directories(segmentation_perf PUBLIC lib/stb/)
target_include_directories(segmentation_perf PUBLIC lib/perfmanager/)
target_link_libraries(segmentation_perf m)

set(SEGMENTATION_PERF_SIMD_SRC
    segmentation_perf_simd/src/main_segmentation.c
    segmentation_perf_simd/src/k-means.c
    segmentation_perf_simd/src/image.c
)

add_executable(segmentation_perf_simd ${SEGMENTATION_PERF_SRC})

target_include_directories(segmentation_perf_simd PUBLIC segmentation_perf_simd/include/)
target_include_directories(segmentation_perf_simd PUBLIC lib/stb/)
target_include_directories(segmentation_perf_simd PUBLIC lib/perfmanager/)
target_link_libraries(segmentation_perf_simd m)

set(SEGMENTATION_POWERCAP_SIMD_SRC
    segmentation_powercap_simd/src/main_segmentation.c
    segmentation_powercap_simd/src/k-means.c
    segmentation_powercap_simd/src/image.c
)

add_executable(segmentation_powercap_simd ${SEGMENTATION_POWERCAP_SIMD_SRC})

target_include_directories(segmentation_powercap_simd PUBLIC segmentation_powercap_simd/include/)
target_include_directories(segmentation_powercap_simd PUBLIC lib/stb/)
target_include_directories(segmentation_powercap_simd PUBLIC lib/powercap/inc)

target_link_libraries(segmentation_powercap_simd m)
target_link_libraries(segmentation_powercap_simd powercap)


set(SEGMENTATION_POWERCAP_SRC
    segmentation_powercap/src/main_segmentation.c
    segmentation_powercap/src/k-means.c
    segmentation_powercap/src/image.c
)

add_executable(segmentation_powercap ${SEGMENTATION_POWERCAP_SRC})

target_include_directories(segmentation_powercap PUBLIC segmentation_powercap/include/)
target_include_directories(segmentation_powercap PUBLIC lib/stb/)
target_include_directories(segmentation_powercap PUBLIC lib/powercap/inc)

target_link_libraries(segmentation_powercap m)
target_link_libraries(segmentation_powercap powercap)
