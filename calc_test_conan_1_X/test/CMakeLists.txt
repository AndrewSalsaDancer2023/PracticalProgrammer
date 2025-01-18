set(CONAN_DISABLE_CHECK_COMPILER TRUE)
#Подключаем сгенерированный скрипт conanbuildinfo.cmake, созданный Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#Выполняем макрос из conanbuildinfo.cmake, который настроит СMake на работу с библиотеками, установленными Conan
conan_basic_setup()


find_package(GTest REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(unit_tests
               arithmetic_test.cpp logic_test.cpp
               )
target_link_libraries(unit_tests PRIVATE arithmetic logic gtest pthread)

