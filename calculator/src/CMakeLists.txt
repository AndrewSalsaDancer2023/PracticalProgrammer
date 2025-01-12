cmake_minimum_required(VERSION 3.15)

set(ProjectName Calculator)
project(${ProjectName} CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(arithmetic)
add_subdirectory(logic)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
	message("build debug version of project")
else()
	message("build release version of project")
endif()

message(${CMAKE_BINARY_DIR})

add_executable(${ProjectName} main.cpp)
target_link_libraries(${ProjectName} PRIVATE arithmetic logic)
