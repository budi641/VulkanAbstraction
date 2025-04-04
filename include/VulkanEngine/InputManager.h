#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> // For callback signatures and key codes
#include <glm/glm.hpp>
#include <array>

namespace VulkanEngine {

// Forward declare Engine class
class Engine;

class InputManager {
public:
    InputManager();
    ~InputManager();

    // Prevent copying
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void setupCallbacks(GLFWwindow* window, Engine* engineInstance);
    void processInput(float deltaTime);

    // Getters for state (could be more refined)
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    glm::vec2 getMousePosition() const;
    glm::vec2 getMouseDelta() const;
    bool isMouseCaptured() const;

    // Public setters (might move to private later)
    void setMouseCaptured(GLFWwindow* window, bool captured);

private:
    // Static GLFW callback functions
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

    // Instance data
    Engine* engine = nullptr; // Pointer back to the engine (needed for camera etc.)
    std::array<bool, 1024> keys{}; // Keyboard state
    std::array<bool, 8> mouseButtons{}; // Mouse button state (GLFW_MOUSE_BUTTON_LAST is 7)
    double lastX = 0.0;
    double lastY = 0.0;
    double currentX = 0.0;
    double currentY = 0.0;
    bool firstMouse = true;
    bool mouseCaptured = false;
};

} // namespace VulkanEngine 