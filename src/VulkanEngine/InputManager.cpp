#include "VulkanEngine/InputManager.h"
#include "VulkanEngine/Engine.h" // Include Engine to access camera etc. (indirectly needed by callbacks)

#include <stdexcept> // For runtime_error
#include <iostream> // For debug output (optional)

namespace VulkanEngine {

InputManager::InputManager() {
    // Initialize state if needed (arrays are zero-initialized by default)
}

InputManager::~InputManager() {
    // Nothing specific to clean up here yet
}

void InputManager::setupCallbacks(GLFWwindow* window, Engine* engineInstance) {
    if (!window) {
        throw std::runtime_error("InputManager::setupCallbacks: Window pointer is null!");
    }
    if (!engineInstance) {
         throw std::runtime_error("InputManager::setupCallbacks: Engine pointer is null!");
    }
    this->engine = engineInstance; // Store engine pointer (needed by callbacks indirectly)

    // Store pointer to this InputManager instance in the GLFW window
    glfwSetWindowUserPointer(window, this);

    // Set the GLFW callbacks to our static methods
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    // Initialize mouse position
    glfwGetCursorPos(window, &lastX, &lastY);
    currentX = lastX;
    currentY = lastY;
}

void InputManager::processInput(float deltaTime) {
     // Input *state* is updated by the callbacks.
     // The Engine's processInput will *query* this state.
     // We might add per-frame updates here later (like resetting mouse delta),
     // but the current delta calculation relies on the previous frame's callback state.

    // Update last positions for the *next* frame's delta calculation.
    // This should happen *after* the current frame's delta has been potentially read by getMouseDelta().
    lastX = currentX;
    lastY = currentY;
}

bool InputManager::isKeyPressed(int key) const {
    if (key >= 0 && key < keys.size()) {
        return keys[key];
    }
    return false;
}

bool InputManager::isMouseButtonPressed(int button) const {
     if (button >= 0 && button < mouseButtons.size()) {
        return mouseButtons[button];
    }
    return false;
}

glm::vec2 InputManager::getMousePosition() const {
    return glm::vec2(static_cast<float>(currentX), static_cast<float>(currentY));
}

// Calculate delta based on current and last positions
// Note: Y offset is often reversed depending on coordinate system expectations
glm::vec2 InputManager::getMouseDelta() const {
    if (firstMouse || !mouseCaptured) {
        return glm::vec2(0.0f, 0.0f);
    }
    return glm::vec2(static_cast<float>(currentX - lastX), static_cast<float>(lastY - currentY));
}

bool InputManager::isMouseCaptured() const {
    return mouseCaptured;
}

void InputManager::setMouseCaptured(GLFWwindow* window, bool captured) {
    if (mouseCaptured == captured) return; // No change

    mouseCaptured = captured;
    if (captured) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // Update last position to prevent jump
        glfwGetCursorPos(window, &lastX, &lastY);
        currentX = lastX;
        currentY = lastY;
        firstMouse = true; // Reset firstMouse flag when capturing
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true; // Also reset when releasing? Maybe not needed.
    }
}


// --- Static GLFW Callbacks --- //
// These functions are called by GLFW and need access to the InputManager instance.

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto inputManager = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!inputManager) return;

    if (key >= 0 && key < inputManager->keys.size()) {
        if (action == GLFW_PRESS) {
            inputManager->keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            inputManager->keys[key] = false;
        }
    }

    // Example: Keep Escape key handling separate or move to Engine::processInput
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Example: Toggle mouse capture with a key (e.g., M)
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        inputManager->setMouseCaptured(window, !inputManager->mouseCaptured);
    }
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto inputManager = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!inputManager) return;

     if (button >= 0 && button < inputManager->mouseButtons.size()) {
        if (action == GLFW_PRESS) {
            inputManager->mouseButtons[button] = true;
        } else if (action == GLFW_RELEASE) {
            inputManager->mouseButtons[button] = false;
        }
    }

    // Example: Capture mouse on right-click press
    // Let's keep right-click capture simple for now
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
         if (!inputManager->mouseCaptured) {
            inputManager->setMouseCaptured(window, true);
         }
    }
    // Removed explicit release on right-click release, use 'M' key to toggle.
}

void InputManager::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto inputManager = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!inputManager) return;

    // Update current position regardless of capture state
    inputManager->currentX = xpos;
    inputManager->currentY = ypos;

    if (inputManager->mouseCaptured) {
        if (inputManager->firstMouse) {
            // Set last positions to current on the first frame of capture
            inputManager->lastX = xpos;
            inputManager->lastY = ypos;
            inputManager->firstMouse = false;
        }
        // Delta is calculated in getMouseDelta() when needed
    } else {
         // If not captured, keep lastX/Y updated to avoid jumps when capturing
         inputManager->lastX = xpos;
         inputManager->lastY = ypos;
         inputManager->firstMouse = true; // Reset flag when not captured
    }
}

} // namespace VulkanEngine 