cmake_minimum_required(VERSION 3.24)

project(NihEngine
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------------------------------------------------------------------
# Engine executable sources
# Note: re-run generate.bat whenever you add or remove files.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE NIHENGINE_SOURCES
    "NihEngine/*.cpp"
    "NihEngine/*.h"
)

# ---------------------------------------------------------------------------
# External dependencies
# ---------------------------------------------------------------------------
add_subdirectory(ExternalDependencies)

# ---------------------------------------------------------------------------
# Engine target
# ---------------------------------------------------------------------------
add_executable(NihEngine WIN32 ${NIHENGINE_SOURCES})

target_compile_features(NihEngine PRIVATE cxx_std_20)

target_include_directories(NihEngine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/NihEngine
        ${CMAKE_CURRENT_SOURCE_DIR}/ExternalDependencies/Imgui
)

target_link_libraries(NihEngine
    PRIVATE
        # DirectX / Windows graphics
        d3d12.lib
        dxgi.lib
        dxguid.lib
        uuid.lib
        # Windows core
        kernel32.lib
        user32.lib
        gdi32.lib
        comdlg32.lib
        advapi32.lib
        shell32.lib
        ole32.lib
        oleaut32.lib
        runtimeobject.lib
        # Third-party
        DirectXTK12
)

# ---------------------------------------------------------------------------
# Compile definitions
# ---------------------------------------------------------------------------
target_compile_definitions(NihEngine
    PRIVATE
        $<IF:$<CONFIG:DEBUG>,_DEBUG,NDEBUG>
)

# ---------------------------------------------------------------------------
# Compile options (MSVC)
# ---------------------------------------------------------------------------
target_compile_options(NihEngine
    PRIVATE
        /W4                         # Reasonable warning level (Wall is too noisy for MSVC)
        /permissive-                # Strict conformance
        /Zc:__cplusplus             # Correct __cplusplus macro value
        /Zc:inline                  # Remove unreferenced COMDAT
        /Zc:preprocessor            # Standard-conformant preprocessor
        /EHsc                       # Standard C++ exception handling
        /sdl                        # Additional security checks
        /GR                         # RTTI (needed by DirectXTK12)
        /fp:fast                    # Fast floating-point

        # Disable noisy warnings that are not actionable in this project
        /wd4061 /wd4365 /wd4514 /wd4571 /wd4668 /wd4710 /wd4820 /wd5039 /wd5045
        /wd4265 /wd4625 /wd4626 /wd4986 /wd5204 /wd5220 /wd5264

        # Control-flow enforcement (release only)
        $<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>
)

# EH continuation metadata (MSVC 19.28+ / x64 only)
if((MSVC_VERSION GREATER_EQUAL 1928) AND (CMAKE_SIZEOF_VOID_P EQUAL 8)
   AND ((NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang")) OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0)))
    target_compile_options(NihEngine PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/guard:ehcont>)
    target_link_options(NihEngine   PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/guard:ehcont>)
endif()

# ---------------------------------------------------------------------------
# Link options
# ---------------------------------------------------------------------------
target_link_options(NihEngine
    PRIVATE
        /DYNAMICBASE
        /NXCOMPAT
        $<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>
)

# SAFESEH for 32-bit x86 only
if((CMAKE_SIZEOF_VOID_P EQUAL 4) AND (NOT DIRECTX_ARCH MATCHES "^arm"))
    target_link_options(NihEngine PRIVATE /SAFESEH)
endif()

# ---------------------------------------------------------------------------
# Visual Studio source groups (mirrors folder structure in the IDE)
# ---------------------------------------------------------------------------
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/NihEngine"
    PREFIX "NihEngine"
    FILES ${NIHENGINE_SOURCES}
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()
add_subdirectory(Tests)