#include "VulkanEngine/Window.h"
#include <stdexcept>

namespace VulkanEngine {

Window::Window(int w, int h, std::string title)
    : width(w), height(h), windowTitle(std::move(title))
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

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

// Static callback implementation
void Window::framebufferResizeCallback(GLFWwindow* glfwWin, int w, int h) {
    auto windowInstance = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWin));
    windowInstance->framebufferResized = true;
    windowInstance->width = w;
    windowInstance->height = h;
}

} // namespace VulkanEngine 