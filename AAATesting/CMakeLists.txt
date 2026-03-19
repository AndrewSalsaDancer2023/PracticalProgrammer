# Минимальная версия CMake
cmake_minimum_required(VERSION 3.15)
project(AAATest)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}*.cpp)
add_executable(run_tests main.cpp ${SOURCES})
target_link_libraries(run_tests ${GTEST_LIBRARIES} pthread)
