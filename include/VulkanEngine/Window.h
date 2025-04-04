#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan.h> // Include Vulkan header for VkInstance/VkSurfaceKHR

namespace VulkanEngine {

class Window {
public:
    Window(int width, int height, std::string title);
    ~Window();

    // Delete copy constructor and assignment operator
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const { return glfwWindowShouldClose(window); }
    VkExtent2D getExtent() const { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
    bool wasResized() const { return framebufferResized; }
    void resetResizedFlag() { framebufferResized = false; }
    GLFWwindow* getGLFWwindow() const { return window; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
    void initWindow();

    // Static callback needs access to the Window instance
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    int width;
    int height;
    bool framebufferResized = false;
    std::string windowTitle;
    GLFWwindow* window = nullptr;
};

} // namespace VulkanEngine 