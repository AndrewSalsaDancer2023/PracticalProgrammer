add_subdirectory(arithmetic)
add_subdirectory(logic)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	message("build debug version of project")
else()
	message("build release version of project")
endif()
message(${CMAKE_BINARY_DIR})

add_executable(bootsrtap bootstrap.cpp)
target_link_libraries(bootsrtap PRIVATE arithmetic logic)
