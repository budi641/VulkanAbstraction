# Vulkan Abstraction

A graphics application using Vulkan, GLFW, and GLM.

## Prerequisites

- CMake 3.10 or higher
- Visual Studio 2022 (or any other C++ compiler)
- Vulkan, GLFW, and GLM dependencies (included in the vendor folder)

## Building

### Windows

1. Run the `build.bat` script:
   ```
   build.bat
   ```

2. This will:
   - Create a build directory
   - Generate Visual Studio 2022 project files
   - Open the solution in Visual Studio

3. Build the project in Visual Studio by clicking Build > Build Solution

### Manual Build

1. Create a build directory:
   ```
   mkdir build
   cd build
   ```

2. Generate build files:
   ```
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

3. Build the project:
   ```
   cmake --build . --config Release
   ```

## Project Structure

- `src/` - Source files
- `vendor/` - External dependencies:
  - Vulkan SDK
  - GLFW
  - GLM 