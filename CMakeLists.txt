cmake_minimum_required(VERSION 3.10)
project(VulkanAbstraction VERSION 1.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find Vulkan SDK for shader compiler
find_package(Vulkan REQUIRED COMPONENTS glslc glslangValidator)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found.")
else()
    message(STATUS "Found Vulkan: ${Vulkan_LIBRARIES} (found version \"${Vulkan_VERSION}\") found components: glslc glslangValidator")
    message(STATUS "Found Vulkan shader compiler (glslc): ${Vulkan_GLSLC_EXECUTABLE}")
endif()

# Add source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/VulkanEngine/*.cpp"
    "src/VulkanEngine/Window.cpp"
    "src/VulkanEngine/InputManager.cpp"
    "src/VulkanEngine/Camera.cpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add include directories for the executable
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/Vulkan/Include # Assuming vendor path
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/GLFW/include # Assuming vendor path
    # Add Vulkan include directory found by find_package
    ${Vulkan_INCLUDE_DIRS}
)

# Link with Vulkan (using find_package target)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
message(STATUS "Linking with Vulkan library: ${Vulkan_LIBRARIES}")

# Link with GLFW (use pre-built library)
set(GLFW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/vendor/GLFW/lib-vc2022/glfw3.lib")
if(EXISTS "${GLFW_LIB}")
    target_link_libraries(${PROJECT_NAME} ${GLFW_LIB})
    message(STATUS "GLFW library found: ${GLFW_LIB}")
else()
    message(FATAL_ERROR "GLFW library not found: ${GLFW_LIB}")
endif()

# GLM is header-only, so we only need to include it (done above)

# Define shader files
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(VERTEX_SHADER ${SHADER_DIR}/shader.vert)
set(FRAGMENT_SHADER ${SHADER_DIR}/shader.frag)
set(VERTEX_SHADER_OUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/shader.vert.spv)
set(FRAGMENT_SHADER_OUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/shader.frag.spv)

# Create output directory for shaders
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# Add custom command to compile vertex shader
add_custom_command(
    OUTPUT ${VERTEX_SHADER_OUT}
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${VERTEX_SHADER} -o ${VERTEX_SHADER_OUT}
    DEPENDS ${VERTEX_SHADER}
    COMMENT "Compiling vertex shader ${VERTEX_SHADER} -> ${VERTEX_SHADER_OUT}"
)

# Add custom command to compile fragment shader
add_custom_command(
    OUTPUT ${FRAGMENT_SHADER_OUT}
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${FRAGMENT_SHADER} -o ${FRAGMENT_SHADER_OUT}
    DEPENDS ${FRAGMENT_SHADER}
    COMMENT "Compiling fragment shader ${FRAGMENT_SHADER} -> ${FRAGMENT_SHADER_OUT}"
)

# Add the compiled shaders as dependencies to the executable
add_custom_target(Shaders ALL DEPENDS ${VERTEX_SHADER_OUT} ${FRAGMENT_SHADER_OUT})
add_dependencies(${PROJECT_NAME} Shaders)

# Set output directories for executable
# CMAKE_RUNTIME_OUTPUT_DIRECTORY defaults to CMAKE_BINARY_DIR/<Config> or CMAKE_BINARY_DIR
# We will copy shaders relative to the executable later.

# Set startup project for Visual Studio (optional, uncomment if needed)
# set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Copy compiled shaders to the executable output directory AFTER build
# Using generator expressions to get the executable location
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${VERTEX_SHADER_OUT}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/shader.vert.spv"
    COMMAND ${CMAKE_COMMAND} -E copy "${FRAGMENT_SHADER_OUT}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/shader.frag.spv"
    COMMENT "Copying compiled shaders to $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
) 