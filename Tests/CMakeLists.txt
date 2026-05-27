include(FetchContent)

# ---------------------------------------------------------------------------
# GoogleTest
# ---------------------------------------------------------------------------
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
# Force shared CRT on Windows so it matches the engine target
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ---------------------------------------------------------------------------
# Test sources
# Note: re-run generate.bat whenever you add or remove files.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE TEST_SOURCES
    "*.cpp"
)

# ---------------------------------------------------------------------------
# Test executable
# ---------------------------------------------------------------------------
add_executable(NihEngineTests ${TEST_SOURCES})

target_compile_features(NihEngineTests PRIVATE cxx_std_20)

target_include_directories(NihEngineTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../NihEngine
)

target_link_libraries(NihEngineTests
    PRIVATE
        GTest::gtest_main
)

# ---------------------------------------------------------------------------
# Test discovery
# ---------------------------------------------------------------------------
include(GoogleTest)
gtest_discover_tests(NihEngineTests DISCOVERY_MODE PRE_TEST)