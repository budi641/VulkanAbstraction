#include "VulkanEngine/Engine.h" // Use relative path from include dir

// Include Vulkan Headers directly in CPP files
#include <vulkan/vulkan.h>

// Include libraries used in implementation
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <cstring> // For strcmp

#include <glm/gtc/matrix_transform.hpp>

// --- Define Global Proxy Functions (Matching Declarations in Engine.h) ---
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        std::cerr << "[WARN] vkCreateDebugUtilsMessengerEXT extension not present!" << std::endl;
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    } else {
         std::cerr << "[WARN] vkDestroyDebugUtilsMessengerEXT extension not present!" << std::endl;
    }
}
// --- End Global Proxy Functions ---

namespace VulkanEngine {

//-------------------------------------------------
// Static Helper Function Implementations (Engine-specific)
//-------------------------------------------------

// readFile remains static within Engine
std::vector<char> Engine::readFile(const std::string& filename) {
    // Adjusted path logic: Assume shaders are relative to executable now
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        // Use cerr for errors
        std::cerr << "Failed to open file: " << filename << std::endl;
        throw std::runtime_error("Failed to open file: " + filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

// debugCallback remains static within Engine
VKAPI_ATTR VkBool32 VKAPI_CALL Engine::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    // Filter out less important messages if desired
    // if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    // }
    return VK_FALSE;
}

//-------------------------------------------------
// Vertex Struct Implementation
//-------------------------------------------------
VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

    // Position attribute
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    // Color attribute
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}

//-------------------------------------------------
// Engine Class Implementation
//-------------------------------------------------

Engine::Engine(int width, int height, const std::string& title)
    : windowWidth(width), windowHeight(height), windowTitle(title),
      // Initialize hardcoded vertex/index data here
      vertices({
          // Front face (red)
          {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
          {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
          {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
          {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
          // Back face (green)
          {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
          {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
          {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
          {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
          // Left face (blue)
          {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
          {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
          {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
          {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
          // Right face (yellow)
          {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
          {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
          {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
          {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
          // Top face (magenta)
          {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          // Bottom face (cyan)
          {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
          {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
          {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
          {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}}
      }),
      indices({
          // Front face
          0, 1, 2, 2, 3, 0,
          // Back face
          4, 7, 6, 6, 5, 4,
          // Left face
          8, 9, 10, 10, 11, 8,
          // Right face
          12, 13, 14, 14, 15, 12,
          // Top face
          16, 17, 18, 18, 19, 16,
          // Bottom face
          20, 21, 22, 22, 23, 20
      })
{
    // Constructor body (if needed)
}

Engine::~Engine() {
    // Destructor body (cleanup is called explicitly in run for now)
}

void Engine::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void Engine::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Resizing handled by recreateSwapChain
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Default is true

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window!");
    }

    // Store pointer to this Engine instance in the GLFW window
    glfwSetWindowUserPointer(window, this);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    // Initialize mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    firstMouse = true; // Ensure first mouse movement doesn't cause jump

}

// --- GLFW Callbacks --- (Must be static or global)

void Engine::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
    engine->framebufferResized = true;
    // Store new size for potential use (e.g., aspect ratio in projection)
    engine->windowWidth = width;
    engine->windowHeight = height;
}

void Engine::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            engine->keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            engine->keys[key] = false;
        }
    }
    // Close window on ESC press
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void Engine::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            engine->mouseCaptured = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // Reset mouse position on capture to avoid jump
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            engine->lastX = static_cast<float>(xpos);
            engine->lastY = static_cast<float>(ypos);
            engine->firstMouse = true; // Reset first mouse flag
        } else if (action == GLFW_RELEASE) {
            engine->mouseCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void Engine::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (engine->mouseCaptured) {
        if (engine->firstMouse) {
            engine->lastX = static_cast<float>(xpos);
            engine->lastY = static_cast<float>(ypos);
            engine->firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos) - engine->lastX;
        float yoffset = engine->lastY - static_cast<float>(ypos); // Reversed

        engine->lastX = static_cast<float>(xpos);
        engine->lastY = static_cast<float>(ypos);

        xoffset *= engine->mouseSensitivity;
        yoffset *= engine->mouseSensitivity;

        engine->yaw += xoffset;
        engine->pitch += yoffset;

        // Constrain pitch
        if (engine->pitch > 89.0f) engine->pitch = 89.0f;
        if (engine->pitch < -89.0f) engine->pitch = -89.0f;

        engine->updateCameraVectors();
    }
}

// --- Input Processing --- (Member function)

void Engine::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void Engine::processInput() {
    float currentSpeed = cameraSpeed * deltaTime; // Apply deltaTime

    if (keys[GLFW_KEY_W]) {
        cameraPos += currentSpeed * cameraFront;
    }
    if (keys[GLFW_KEY_S]) {
        cameraPos -= currentSpeed * cameraFront;
    }
    if (keys[GLFW_KEY_A]) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;
    }
    if (keys[GLFW_KEY_D]) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;
    }
    if (keys[GLFW_KEY_SPACE]) {
        cameraPos += currentSpeed * cameraUp;
    }
    if (keys[GLFW_KEY_LEFT_CONTROL] || keys[GLFW_KEY_RIGHT_CONTROL]) {
        cameraPos -= currentSpeed * cameraUp;
    }
}

// --- Vulkan Initialization --- (Member function)

void Engine::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();       // Depends on swap chain format
    createDescriptorSetLayout();
    createGraphicsPipeline(); // Depends on render pass, layout, swap chain extent
    createFramebuffers();     // Depends on image views, render pass, swap chain extent
    createCommandPool();      // Depends on logical device, queue families
    createVertexBuffer();     // Depends on logical device, physical device, command pool, queue
    createIndexBuffer();      // Depends on logical device, physical device, command pool, queue
    createUniformBuffers();   // Depends on logical device, physical device
    createDescriptorPool();   // Depends on logical device
    createDescriptorSets();   // Depends on logical device, pool, layout, uniform buffers
    createCommandBuffers();   // Depends on logical device, command pool
    createSyncObjects();      // Depends on logical device
}

// --- Main Loop & Cleanup --- (Member functions)

void Engine::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents();
        processInput(); // Handle movement based on deltaTime
        drawFrame();
    }

    // Wait for the device to finish operations before cleanup
    vkDeviceWaitIdle(device);
}

void Engine::cleanup() {
    cleanupSwapChain(); // Clean swap chain resources first

    // Destroy descriptor pool (implicitly destroys sets)
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    // Destroy descriptor set layout
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    // Destroy uniform buffers and memory
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // No need to unmap if persistently mapped (driver handles on FreeMemory)
        // vkUnmapMemory(device, uniformBuffersMemory[i]); // Removed based on comment in main.cpp
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    // Destroy index buffer and memory
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    // Destroy vertex buffer and memory
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    // Destroy synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    // Destroy command pool (implicitly destroys command buffers)
    vkDestroyCommandPool(device, commandPool, nullptr);

    // Destroy logical device
    vkDestroyDevice(device, nullptr);

    // Destroy debug messenger (if enabled)
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    // Destroy surface
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // Destroy instance
    vkDestroyInstance(instance, nullptr);

    // Destroy window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}

// --- Vulkan Implementation Details (Private Methods) ---
// (Copy implementations from main.cpp VulkanApplication class here)
// Remember to replace member access with `this->` or just the name,
// and adjust static function calls if necessary.

void Engine::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = windowTitle.c_str(); // Use windowTitle
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "VulkanEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{}; // Define outside if block
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        // Populate debug messenger create info
        debugCreateInfo = {}; // Zero initialize
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pUserData = nullptr; // Optional: pass 'this' if callback needs instance data

        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance!");
    }
}

bool Engine::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            std::cerr << "Validation layer not found: " << layerName << std::endl;
            return false;
        }
    }
    return true;
}

std::vector<const char*> Engine::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Optional: Enumerate and print available extensions for debugging
    /*
    uint32_t availableExtCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtCount, nullptr);
    std::vector<VkExtensionProperties> availableExt(availableExtCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtCount, availableExt.data());
    std::cout << "Available instance extensions:\n";
    for(const auto& ext : availableExt) {
        std::cout << '\t' << ext.extensionName << '\n';
    }
    std::cout << "Required instance extensions:\n";
    for(const auto* extName : extensions) {
        std::cout << '\t' << extName << '\n';
    }
    */

    return extensions;
}

void Engine::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Or 'this'

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void Engine::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void Engine::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Prefer discrete GPU
    for (const auto& dev : devices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(dev, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && isDeviceSuitable(dev)) {
            physicalDevice = dev;
             std::cout << "Picked Discrete GPU: " << properties.deviceName << std::endl;
            return;
        }
    }

    // Fallback to any suitable GPU
    for (const auto& dev : devices) {
        if (isDeviceSuitable(dev)) {
            physicalDevice = dev;
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(dev, &properties);
            std::cout << "Picked GPU: " << properties.deviceName << std::endl;
            return;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool Engine::isDeviceSuitable(VkPhysicalDevice queryDevice) {
    QueueFamilyIndices indices = findQueueFamilies(queryDevice);
    bool extensionsSupported = checkDeviceExtensionSupport(queryDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(queryDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // Optional: Check for required features
    // VkPhysicalDeviceFeatures supportedFeatures;
    // vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    // return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices Engine::findQueueFamilies(VkPhysicalDevice queryDevice) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(queryDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(queryDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        // Check for graphics support
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        // Check for presentation support
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(queryDevice, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

bool Engine::checkDeviceExtensionSupport(VkPhysicalDevice queryDevice) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(queryDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(queryDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    if (!requiredExtensions.empty()) {
        std::cerr << "Missing required device extensions:" << std::endl;
        for(const auto& ext : requiredExtensions) {
            std::cerr << "\t" << ext << std::endl;
        }
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails Engine::querySwapChainSupport(VkPhysicalDevice queryDevice) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(queryDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(queryDevice, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(queryDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(queryDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(queryDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        // Prefer SRGB format for more accurate colors
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    // Fallback to the first available format
    return availableFormats[0];
}

VkPresentModeKHR Engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        // Prefer Mailbox for triple buffering (low latency, no tearing)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    // FIFO is guaranteed to be available (vsync)
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Engine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        // Window manager dictates extent
        return capabilities.currentExtent;
    } else {
        // We can choose extent within limits
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // Clamp values to allowed range
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Engine::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{}; // Empty for now, enable features as needed
    // Example: deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // For compatibility with older implementations
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    // Get queue handles
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void Engine::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // Request one more image than minimum for smoother operation (allows triple buffering with Mailbox)
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // Clamp image count within allowed range (maxImageCount = 0 means no limit)
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // 1 for non-stereoscopic
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Use image as color buffer

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndicesArray[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        // Images used by multiple queues (graphics and present)
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
    } else {
        // Image owned by one queue family at a time
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // Use current transform (no rotation/flip)
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Don't blend with window system
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // Discard rendering outside screen bounds
    createInfo.oldSwapchain = VK_NULL_HANDLE; // No old swapchain on initial creation

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    // Retrieve swap chain images
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    // Store swap chain format and extent for later use
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void Engine::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        // Default component mapping (RGBA -> RGBA)
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Describe the image's purpose and which part to access
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

VkShaderModule Engine::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    // Need to cast pointer due to Vulkan API expecting uint32_t*
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }
    return shaderModule;
}

void Engine::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;       // Format of the attachment
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;     // No multisampling for now
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear framebuffer before drawing
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;// Store result after drawing
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Not using stencil
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;     // Layout before render pass
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Layout after for presentation

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // Index of the attachment in the pAttachments array
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout during subpass

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // This is a graphics subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // Reference to the color attachment
    // pDepthStencilAttachment = nullptr; // No depth/stencil for now

    // Add dependency to ensure render pass waits for image acquisition
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Implicit subpass before render pass
    dependency.dstSubpass = 0;                   // Our first (and only) subpass
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Wait at this stage
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Before this stage
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;        // Need write access

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void Engine::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0; // Corresponds to 'binding = 0' in shader
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1; // One UBO
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // UBO used in vertex shader
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    // Add bindings for textures, etc. here later
    // VkDescriptorSetLayoutBinding samplerLayoutBinding{}; ...

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {uboLayoutBinding /*, samplerLayoutBinding */};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void Engine::createGraphicsPipeline() {
    // --- Shader Stages --- //
    auto vertShaderCode = readFile("shaders/shader.vert.spv");
    auto fragShaderCode = readFile("shaders/shader.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // Entry point function

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // --- Vertex Input --- //
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // --- Input Assembly --- //
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Draw triangles
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // --- Viewport and Scissor --- //
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // --- Rasterizer --- //
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // Don't clamp fragments beyond near/far planes
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Don't discard geometry
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Fill polygons
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // Cull back faces
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Assuming vertices are actually clockwise for the outside
    rasterizer.depthBiasEnable = VK_FALSE;
    // rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    // rasterizer.depthBiasClamp = 0.0f; // Optional
    // rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // --- Multisampling --- // (Disabled for now)
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    // multisampling.minSampleShading = 1.0f; // Optional
    // multisampling.pSampleMask = nullptr; // Optional
    // multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    // multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // --- Depth and Stencil Testing --- // (Disabled for now)
    // VkPipelineDepthStencilStateCreateInfo depthStencil{};
    // depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // depthStencil.depthTestEnable = VK_TRUE;
    // depthStencil.depthWriteEnable = VK_TRUE;
    // depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    // ... (configure stencil if needed)

    // --- Color Blending --- // (Disabled for now)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    // colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    // colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    // colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    // colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    // colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    // colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE; // Use blend factors, not bitwise logic op
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    // colorBlending.blendConstants[0] = 0.0f; // Optional
    // colorBlending.blendConstants[1] = 0.0f; // Optional
    // colorBlending.blendConstants[2] = 0.0f; // Optional
    // colorBlending.blendConstants[3] = 0.0f; // Optional

    // --- Dynamic States --- // (Viewport/Scissor could be dynamic)
    // std::vector<VkDynamicState> dynamicStates = {
    //     VK_DYNAMIC_STATE_VIEWPORT,
    //     VK_DYNAMIC_STATE_SCISSOR
    // };
    // VkPipelineDynamicStateCreateInfo dynamicState{};
    // dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    // dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    // dynamicState.pDynamicStates = dynamicStates.data();

    // --- Pipeline Layout --- //
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    // pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    // pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    // --- Graphics Pipeline Creation --- //
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Set later if using depth buffer
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Set later if using dynamic states
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // Index of the subpass where this pipeline will be used
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional: derive from another pipeline
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    // --- Cleanup Shader Modules --- //
    // Shader modules are not needed after pipeline creation
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void Engine::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 1> attachments = {
            swapChainImageViews[i]
            // Add depth image view here if using depth buffer
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void Engine::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // Allow command buffers to be reset individually
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

uint32_t Engine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        // Check if the bit for this memory type is set in the filter
        // AND if the memory type has ALL the required property flags
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void Engine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only used by graphics queue

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        // Clean up buffer if memory allocation failed
        vkDestroyBuffer(device, buffer, nullptr);
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    // Associate the allocated memory with the buffer
    // The final '0' is the offset within the memory allocation
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

VkCommandBuffer Engine::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Engine::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Submit to the graphics queue and wait for completion
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    // Free the temporary command buffer
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Engine::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}


void Engine::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // Create a temporary staging buffer (CPU accessible)
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    // Map memory, copy data, and unmap
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Create the final vertex buffer (GPU local)
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 vertexBuffer, vertexBufferMemory);

    // Copy data from staging buffer to device local buffer
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    // Clean up staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Engine::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Engine::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // Host coherent often simplifies updates
                     uniformBuffers[i], uniformBuffersMemory[i]);

        // Persistently map the buffers for efficient updates
        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        // Note: No need to explicitly unmap persistently mapped memory before freeing.
    }
}

void Engine::createDescriptorPool() {
    // Describe the types and counts of descriptors needed
    std::array<VkDescriptorPoolSize, 1> poolSizes{}; // Expand if more types needed (e.g., textures)
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // One UBO per frame

    // poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_TEXTURES);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // Max number of sets to allocate
    // poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // If sets need to be freed individually

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void Engine::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    // Configure the descriptors within each set
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // Add VkDescriptorImageInfo for textures here...

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{}; // Expand for textures

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];         // Set to update
        descriptorWrites[0].dstBinding = 0;                   // Binding index (matches layout)
        descriptorWrites[0].dstArrayElement = 0;              // Index in array (if descriptor is array)
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;         // Buffer info
        // descriptorWrites[0].pImageInfo = nullptr;           // Optional
        // descriptorWrites[0].pTexelBufferView = nullptr;     // Optional

        // Add write for texture sampler...
        // descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // descriptorWrites[1].dstSet = descriptorSets[i];
        // descriptorWrites[1].dstBinding = 1; // Texture binding
        // ... (configure image info)

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Engine::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Can be submitted directly to queue
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void Engine::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled so first frame doesn't wait indefinitely

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void Engine::drawFrame() {
    // 1. Wait for the previous frame using the fence for this frame index
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    // Fence is reset manually after acquiring image

    // 2. Acquire an image from the swap chain
    uint32_t imageIndex;
    // Use the semaphore for this frame index to signal when the image is available
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    // Handle swap chain recreation if necessary
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return; // Skip rest of frame draw
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        // Suboptimal is okay, but other errors are fatal
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    // Now that we know we'll submit work for this frame, reset the fence
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // 3. Record the command buffer for the acquired image index
    vkResetCommandBuffer(commandBuffers[currentFrame], 0); // Reset before recording
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    // 4. Update uniform buffer for the current frame
    updateUniformBuffer(currentFrame);

    // 5. Submit the command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Specify which semaphores to wait on before execution begins
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // Wait at this stage
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // Specify which command buffers to submit
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    // Specify which semaphores to signal once the command buffer finishes
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Submit to the graphics queue, using the fence for this frame
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    // 6. Presentation
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Specify which semaphores to wait on before presentation can happen
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores; // Wait for rendering to finish

    // Specify the swap chain and image index to present
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    // Handle swap chain recreation or errors during presentation
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false; // Reset flag after handling
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    // Advance to the next frame index
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Engine::updateUniformBuffer(uint32_t currentImage) {
    // static auto startTime = std::chrono::high_resolution_clock::now();
    // auto currentTime = std::chrono::high_resolution_clock::now();
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    float time = static_cast<float>(glfwGetTime()); // Use GLFW time for consistency

    UniformBufferObject ubo{};
    // Rotate model matrix
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z

    // View matrix based on camera position and orientation
    ubo.view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Projection matrix
    float aspect = swapChainExtent.width / (float)swapChainExtent.height;
    ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f); // Increased far plane

    // GLM was designed for OpenGL, where Y coordinate points up.
    // Vulkan's Y coordinate points down. Invert the Y-axis scaling factor.
    ubo.proj[1][1] *= -1;

    // Copy data to the mapped buffer for the current frame
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Engine::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // beginInfo.flags = 0; // Optional
    // beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Begin Render Pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex]; // Target framebuffer
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    // Define clear color
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // Black background
    // Add clear depth/stencil if using depth buffer
    // VkClearValue clearDepth = {{{1.0f, 0}}};
    // std::array<VkClearValue, 2> clearValues = {clearColor, clearDepth};
    renderPassInfo.clearValueCount = 1; // Update if depth added
    renderPassInfo.pClearValues = &clearColor; // Use clearValues.data() if depth added

    // Start the render pass
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind the graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // Bind vertex buffer
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    // Bind index buffer
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // Bind descriptor sets for this frame (contains UBO)
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    // Set dynamic viewport and scissor (if pipeline configured for dynamic states)
    /*
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    */

    // Draw indexed primitives
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    // Parameters: indexCount, instanceCount, firstIndex, vertexOffset, firstInstance

    // End the render pass
    vkCmdEndRenderPass(commandBuffer);

    // Finish recording
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void Engine::cleanupSwapChain() {
    // Destroy framebuffers
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    // Destroy graphics pipeline
    vkDestroyPipeline(device, graphicsPipeline, nullptr);

    // Destroy pipeline layout
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    // Destroy render pass
    vkDestroyRenderPass(device, renderPass, nullptr);

    // Destroy image views
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    // Destroy swap chain itself
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void Engine::recreateSwapChain() {
    // Handle minimization (pause rendering)
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents(); // Wait until window is restored
    }

    // Wait for device to finish current operations before recreating resources
    vkDeviceWaitIdle(device);

    // 1. Cleanup existing swap chain resources
    cleanupSwapChain();

    // 2. Recreate swap chain and dependent resources
    createSwapChain();       // Recreate swap chain itself
    createImageViews();      // Recreate image views for new images
    createRenderPass();      // Recreate render pass (format might change, though unlikely here)
    createGraphicsPipeline();// Recreate pipeline (depends on extent, render pass)
    createFramebuffers();    // Recreate framebuffers (depend on image views, extent, render pass)

    // Uniform buffers and descriptor sets usually don't need recreation unless UBO size/layout changes.
    // Command buffers will be re-recorded in the next drawFrame.
}


} // namespace VulkanEngine
