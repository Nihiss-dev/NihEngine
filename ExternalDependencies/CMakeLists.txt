# Include DirectXTK12 via CMake
include(FetchContent)
FetchContent_Declare(
  DirectXTK12
  GIT_REPOSITORY https://github.com/microsoft/DirectXTK12.git
  GIT_TAG        feb2023
)
FetchContent_MakeAvailable(DirectXTK12)

