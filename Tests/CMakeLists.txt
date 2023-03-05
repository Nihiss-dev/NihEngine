# Include Google Test via CMake
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

cmake_minimum_required(VERSION 3.10)

project(test_me)

enable_testing()

#find_package(GTest REQUIRED)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")
add_executable(runUnitTests ${TEST_SOURCES})

include(GoogleTest)
target_link_libraries(runUnitTests GTest::gtest_main)

gtest_discover_tests(runUnitTests)