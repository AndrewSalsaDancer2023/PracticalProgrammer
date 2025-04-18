include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
               arithmetic_test.cpp logic_test.cpp variable_test.cpp algo_test.cpp run_test.cpp
               )
target_link_libraries(unit_tests PRIVATE arithmetic logic gtest_main pthread tbb)

include(coverage)
AddCoverage(unit_tests)


include(GoogleTest)
gtest_discover_tests(unit_tests)

include(Valgrind)
#AddMemeoryCheck(unit_tests)
AddThreadCheck(unit_tests)
