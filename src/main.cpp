#include "VulkanEngine/Engine.h" // Include the new engine header

#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main() {
    // Create an instance of the engine
    VulkanEngine::Engine engine(1024, 768, "Vulkan Engine Refactored"); // Example: Use different size/title

    try {
        // Run the engine
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
} 