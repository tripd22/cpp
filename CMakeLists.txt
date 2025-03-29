cmake_minimum_required(VERSION 3.10)

project(cpp)

set(CMAKE_CXX_STANDARD 23)

add_executable(branchless branchless/benchmark_main.cc)

find_package(benchmark REQUIRED)
target_link_libraries(cpp benchmark::benchmark)