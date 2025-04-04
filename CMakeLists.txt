cmake_minimum_required(VERSION 3.10)
project(VulkanAbstraction VERSION 1.0)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add include directories for the executable
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/Vulkan/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/GLFW/include
)

# Link with Vulkan (direct approach - explicitly look for vulkan-1.lib)
set(VULKAN_LIB "${CMAKE_CURRENT_SOURCE_DIR}/vendor/Vulkan/Lib/vulkan-1.lib")
if(EXISTS "${VULKAN_LIB}")
    target_link_libraries(${PROJECT_NAME} ${VULKAN_LIB})
    message(STATUS "Vulkan library found: ${VULKAN_LIB}")
else()
    message(FATAL_ERROR "Vulkan library not found: ${VULKAN_LIB}")
endif()

# Link with GLFW (use pre-built library)
set(GLFW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/vendor/GLFW/lib-vc2022/glfw3.lib")
if(EXISTS "${GLFW_LIB}")
    target_link_libraries(${PROJECT_NAME} ${GLFW_LIB})
    message(STATUS "GLFW library found: ${GLFW_LIB}")
else()
    message(FATAL_ERROR "GLFW library not found: ${GLFW_LIB}")
endif()

# GLM is header-only, so we only need to include it
# (already included above)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}) 