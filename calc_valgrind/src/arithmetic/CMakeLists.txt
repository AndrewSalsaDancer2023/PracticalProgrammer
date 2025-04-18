add_library(arithmetic SHARED arithmetic.cpp calculator.cpp run.cpp)
target_include_directories(arithmetic PUBLIC .)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
  target_compile_options(arithmetic PRIVATE --coverage)
  target_link_options(arithmetic PUBLIC --coverage)
  add_custom_command(TARGET arithmetic PRE_BUILD COMMAND
                     find ${CMAKE_BINARY_DIR} -type f
                     -name '*.gcda' -exec rm {} +)
endif()
