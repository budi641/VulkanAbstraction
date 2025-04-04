#include <vulkan/vulkan.hpp> // Include C++ wrappers
#include "VulkanEngine/Window.h"
#include <stdexcept>
#include <utility> // For std::move

namespace VulkanEngine {

Window::Window(int w, int h, const std::string& title)
    : width(w), height(h), windowTitle(title)
{
    initWindow();
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Resizing handled by recreateSwapChain in Engine for now
    // We still need the callback to update width/height and set the flag
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window!");
    }

    // Store pointer to this Window instance in the GLFW window
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    // Input callbacks will be set here later when InputManager is created
    // For now, they remain in Engine, but we'll need to pass the Engine pointer
    // or create a dedicated InputManager.
}

void Window::createWindowSurface(vk::Instance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

// Static callback implementation
void Window::framebufferResizeCallback(GLFWwindow* glfwWin, int w, int h) {
    // Get the Window instance via the user pointer
    auto* thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(glfwWin));
    if (thisWindow) {
        // Access members through the obtained pointer
        thisWindow->framebufferResized = true;
        thisWindow->width = w;
        thisWindow->height = h;
    }
}

vk::Extent2D Window::getExtent() const {
    int currentWidth, currentHeight;
    glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
    return vk::Extent2D{
        static_cast<uint32_t>(currentWidth),
        static_cast<uint32_t>(currentHeight)
    };
}

float Window::getAspectRatio() const {
    vk::Extent2D extent = getExtent();
    // Prevent division by zero if window minimized
    if (extent.height == 0) {
        return 1.0f; // Or some default aspect ratio
    }
    return static_cast<float>(extent.width) / static_cast<float>(extent.height);
}

} // namespace VulkanEngine 