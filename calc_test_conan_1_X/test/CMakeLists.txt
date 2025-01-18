if(NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
	message(FATAL_ERROR "This CMake file works on linux systems only! CMake will exit!" )
endif()
set(CONAN_DISABLE_CHECK_COMPILER TRUE)
#Подключение сгенерированного скрипта conanbuildinfo.cmake, который создан Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#Выполнение макроса из conanbuildinfo.cmake, который настраивает СMake на работу с библиотеками, которые установил Conan
conan_basic_setup()


find_package(GTest REQUIRED)
if(GTest_FOUND)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	if(NOT Threads_FOUND)
		message( FATAL_ERROR "pthread not found, CMake will exit!" )
	endif()	
else()
	message(FATAL_ERROR "GTest not found, CMake will exit!" )
endif()



add_executable(unit_tests
               arithmetic_test.cpp logic_test.cpp
               )
target_link_libraries(unit_tests PRIVATE arithmetic logic gtest pthread)

