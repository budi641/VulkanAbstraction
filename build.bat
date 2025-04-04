@echo off
if not exist "build" mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% == 0 (
    echo CMake generation successful. Opening Visual Studio solution...
    start VulkanAbstraction.sln
) else (
    echo CMake generation failed with error code %ERRORLEVEL%
    pause
)
cd .. 