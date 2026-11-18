cmake_minimum_required(VERSION 3.10)

project(allocator_benchmark CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим установленный google benchmark
find_package(benchmark REQUIRED)

add_executable(my_benchmark main.cpp)

# Линкуем проект с библиотекой
target_link_libraries(my_benchmark PRIVATE benchmark::benchmark)

