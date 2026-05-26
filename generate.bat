@echo off
setlocal

set BUILD_DIR=build
set GENERATOR=Visual Studio 18 2026
set ARCH=x64

echo Cleaning build directory...
if exist %BUILD_DIR% (
    rd /s /q %BUILD_DIR%
)

echo Generating solution...
cmake -S . -B %BUILD_DIR% -G "%GENERATOR%" -A %ARCH%

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] CMake configuration failed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Solution generated successfully: %BUILD_DIR%\NihEngine.sln
pause