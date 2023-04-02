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

SET(TEST_EXE NihEngineTest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES "*.cpp")
add_executable(${TEST_EXE} ${TEST_SOURCES})

include(GoogleTest)
target_include_directories(${TEST_EXE} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../NihEngine)
target_link_libraries(${TEST_EXE} GTest::gtest_main)

gtest_discover_tests(${TEST_EXE})