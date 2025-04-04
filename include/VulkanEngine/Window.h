#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan.h> // Include Vulkan header for VkInstance/VkSurfaceKHR

namespace VulkanEngine {

class Engine; // Forward declaration

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    // Delete copy constructor and assignment operator
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const { return glfwWindowShouldClose(window); }
    vk::Extent2D getExtent() const;
    bool wasResized() const { return framebufferResized; }
    void resetResizedFlag() { framebufferResized = false; }
    GLFWwindow* getGLFWwindow() const { return window; }

    void createWindowSurface(vk::Instance instance, VkSurfaceKHR* surface);

    const std::string& getTitle() const { return windowTitle; }
    float getAspectRatio() const;

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