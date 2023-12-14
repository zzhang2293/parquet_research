cmake_minimum_required(VERSION 3.18)
project(parcook)

set(CMAKE_CXX_STANDARD 17)

option(PARCOOK_BUILD_TESTS "Build Parcook tests" ON)
option(PARCOOK_BUILD_BENCHMARKS "Build Parcook benchmarks" ON)

find_package(Arrow 12.0.0 REQUIRED)
find_package(Parquet 12.0.0 REQUIRED)

add_library(
        parcook
        src/aggregation.cpp
        src/arithmetic.cpp
        src/selection.cpp
        src/common.cpp
)

target_include_directories(parcook PRIVATE include)

target_link_libraries(
        parcook
        Arrow::arrow_static
        Parquet::parquet_static
)

if (PARCOOK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()

if (PARCOOK_BUILD_BENCHMARKS)
    add_subdirectory(bench)
endif ()
