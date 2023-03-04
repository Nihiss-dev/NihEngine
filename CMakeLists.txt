cmake_minimum_required(VERSION 3.15)

project(NihEngine)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES "NihEngine/*.cpp")

add_executable(NihEngine ${SOURCES})

target_include_directories(NihEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/NihEngine)

set_target_properties(NihEngine PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(NihEngine PRIVATE User32 Gdi32)