if(NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
	message(FATAL_ERROR "This CMake file works on linux systems only! CMake will exit!" )
endif()

find_package(GTest REQUIRED)
if(NOT GTest_FOUND)
	message(FATAL_ERROR "GTest not found, CMake will exit!" )
endif()

add_executable(unit_tests arithmetic_test.cpp logic_test.cpp)
               
target_link_libraries(unit_tests PRIVATE arithmetic logic gtest::gtest)
