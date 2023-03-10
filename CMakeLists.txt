cmake_minimum_required(VERSION 3.15)

set(DIRECTXTK12_VERSION 1.5.1)

project(NihEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_compile_options(/EHsc)

file(GLOB_RECURSE SOURCES "NihEngine/*.cpp")

add_executable(NihEngine ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d12.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    comdlg32.lib advapi32.lib shell32.lib
    ole32.lib oleaut32.lib
    runtimeobject.lib
)

target_compile_options(${PROJECT_NAME} PRIVATE /Wall /GR /fp:fast "$<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>")
target_link_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>" /DYNAMICBASE /NXCOMPAT)

if((${CMAKE_SIZEOF_VOID_P} EQUAL 4) AND (NOT ${DIRECTX_ARCH} MATCHES "^arm"))
    target_link_options(${PROJECT_NAME} PRIVATE /SAFESEH)
endif()

if((MSVC_VERSION GREATER_EQUAL 1928) AND (CMAKE_SIZEOF_VOID_P EQUAL 8)
    AND ((NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang")) OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0)))
    target_compile_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>:/guard:ehcont>")
    target_link_options(${PROJECT_NAME} PRIVATE "$<$<NOT:$<CONFIG:DEBUG>>:/guard:ehcont>")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
        /sdl /permissive- /Zc:__cplusplus /Zc:inline
        "/wd4061" "/wd4365" "/wd4514" "/wd4571" "/wd4668" "/wd4710" "/wd4820" "/wd5039" "/wd5045"
        "/wd4265" "/wd4625" "/wd4626" "/wd4986" "/wd5204" "/wd5220")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(${PROJECT_NAME} PRIVATE User32 Gdi32)

enable_testing()
add_subdirectory(Tests)