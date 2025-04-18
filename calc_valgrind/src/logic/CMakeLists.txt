add_library(logic SHARED logic.cpp)
target_include_directories(logic PUBLIC .)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
  target_compile_options(logic PRIVATE --coverage)
  target_link_options(logic PUBLIC --coverage)
  add_custom_command(TARGET logic PRE_BUILD COMMAND
                     find ${CMAKE_BINARY_DIR} -type f
                     -name '*.gcda' -exec rm {} +)
endif()
