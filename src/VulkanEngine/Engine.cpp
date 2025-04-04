#include <vulkan/vulkan.hpp>

#include "VulkanEngine/Engine.h"
#include "VulkanEngine/Window.h"
#include "VulkanEngine/InputManager.h"
#include "VulkanEngine/Camera.h"

// Include libraries used in implementation
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <cstring> // For strcmp
#include <memory> // For std::make_unique
#include <limits> // For numeric_limits

#include <glm/gtc/matrix_transform.hpp>

// --- Define Global Proxy Functions (Still Needed for Debug Messenger) ---
// These use the C API types because they bridge to the C loader function
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
// Static/Helper Function Implementations
//-------------------------------------------------

// readFile implementation...
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

// debugCallback implementation (uses C API types due to callback signature)
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
// Engine Class Implementation
//-------------------------------------------------

Engine::Engine(int width, int height, const std::string& title)
    : window(std::make_unique<Window>(width, height, title)),
      inputManager(std::make_unique<InputManager>()), // Initialize InputManager
      camera(glm::vec3(0.0f, 0.0f, 3.0f)), // Initialize Camera
      // Vertex/Index data initialization remains
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
    // Setup InputManager callbacks, passing the window and engine instance
    inputManager->setupCallbacks(window->getGLFWwindow(), this);
}

Engine::~Engine() {
    // Cleanup is handled explicitly by run()
}

void Engine::run() {
    initVulkan();
    mainLoop();
    cleanup();
}

// --- Input Processing using InputManager ---
void Engine::processInput(float deltaTime) {
    // Keyboard movement
    if (inputManager->isKeyPressed(GLFW_KEY_W))
        camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (inputManager->isKeyPressed(GLFW_KEY_S))
        camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (inputManager->isKeyPressed(GLFW_KEY_A))
        camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
    if (inputManager->isKeyPressed(GLFW_KEY_D))
        camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);
     if (inputManager->isKeyPressed(GLFW_KEY_SPACE))
        camera.processKeyboard(Camera_Movement::UP, deltaTime);
    if (inputManager->isKeyPressed(GLFW_KEY_LEFT_CONTROL) || inputManager->isKeyPressed(GLFW_KEY_RIGHT_CONTROL))
        camera.processKeyboard(Camera_Movement::DOWN, deltaTime);

    // Mouse movement (only when captured)
    if (inputManager->isMouseCaptured()) {
        glm::vec2 mouseDelta = inputManager->getMouseDelta();
        camera.processMouseMovement(mouseDelta.x, mouseDelta.y);
    }
}

// --- Main Loop ---
void Engine::mainLoop() {
    float lastFrameTime = 0.0f;

    while (!window->shouldClose()) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents(); // Process events (triggers InputManager callbacks, updates currentX/Y)

        // SWAPPED ORDER:
        // 1. Use input state to update camera (calculates delta using currentX/Y and previous lastX/Y)
        this->processInput(deltaTime);
        // 2. Update InputManager state for the next frame (updates lastX/Y for next delta calc)
        inputManager->processInput(deltaTime);

        // Handle resize (check flag set by Window callback)
        if (window->wasResized()) {
            framebufferResized = true; // Signal drawFrame to handle recreate
            window->resetResizedFlag();
        }

        drawFrame(); // Draw the frame (will handle recreate if framebufferResized is true)
    }
    device.waitIdle(); // Use vk::Device wrapper
}

void Engine::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createDepthResources();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

// --- Vulkan Implementation Details (Updated for vulkan.hpp) ---

void Engine::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo(
        window->getTitle().c_str(), // Use window title
        VK_MAKE_VERSION(1, 0, 0),
        "VulkanEngine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_3 // Request Vulkan 1.3
    );

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo; // Define outside if block
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        // Populate debug messenger create info using C++ types
        debugCreateInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
        debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        // Cast the C callback function pointer to the C++ wrapper type
        debugCreateInfo.pfnUserCallback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback);
        debugCreateInfo.pUserData = nullptr;

        createInfo.pNext = &debugCreateInfo; // Assign address to void*
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pNext = nullptr;
    }

    // Use vk::createInstance
    instance = vk::createInstance(createInfo);
}

bool Engine::checkValidationLayerSupport() {
    // Use vk::enumerateInstanceLayerProperties
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

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
    return extensions;
}

void Engine::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    // Use C++ wrapper type vk::DebugUtilsMessengerCreateInfoEXT
    // but populate it carefully for the C proxy function call
    vk::DebugUtilsMessengerCreateInfoEXT createInfoWrapper;
    createInfoWrapper.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
    createInfoWrapper.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                       vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                       vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfoWrapper.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                   vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                   vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    createInfoWrapper.pfnUserCallback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback);
    createInfoWrapper.pUserData = nullptr;

    // Need to pass a C struct pointer (VkDebugUtilsMessengerCreateInfoEXT*) to the C proxy function.
    // We can obtain this by taking the address of the C++ wrapper object.
    // The memory layout is compatible.
    VkDebugUtilsMessengerCreateInfoEXT* createInfoPtr = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&createInfoWrapper);

    // Call the C proxy function, passing the vk::Instance and the C struct pointer
    if (CreateDebugUtilsMessengerEXT(instance, createInfoPtr, nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT*>(&debugMessenger)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void Engine::createSurface() {
    // Delegate surface creation to the window object, passing vk::Instance
    // The Window class needs to handle the C-style VkSurfaceKHR output
    window->createWindowSurface(instance, reinterpret_cast<VkSurfaceKHR*>(&surface));
}

void Engine::pickPhysicalDevice() {
    // Use instance wrapper
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

    if (devices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    // Prefer discrete GPU
    for (const auto& dev : devices) {
        vk::PhysicalDeviceProperties properties = dev.getProperties();
        if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && isDeviceSuitable(dev)) {
            physicalDevice = dev;
            std::cout << "Picked Discrete GPU: " << properties.deviceName << std::endl;
            return;
        }
    }

    // Fallback to any suitable GPU
    for (const auto& dev : devices) {
        if (isDeviceSuitable(dev)) {
            physicalDevice = dev;
            vk::PhysicalDeviceProperties properties = dev.getProperties();
            std::cout << "Picked GPU: " << properties.deviceName << std::endl;
            return;
        }
    }

    if (!physicalDevice) { // Check if still null
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool Engine::isDeviceSuitable(vk::PhysicalDevice queryDevice) {
    QueueFamilyIndices indices = findQueueFamilies(queryDevice);
    bool extensionsSupported = checkDeviceExtensionSupport(queryDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(queryDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // Optional: Check for required features using queryDevice.getFeatures()
    // vk::PhysicalDeviceFeatures supportedFeatures = queryDevice.getFeatures();
    // return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices Engine::findQueueFamilies(vk::PhysicalDevice queryDevice) {
    QueueFamilyIndices indices;
    // Use device wrapper
    std::vector<vk::QueueFamilyProperties> queueFamilies = queryDevice.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        // Check for graphics support
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        // Check for presentation support using device wrapper
        // Pass the C++ surface object
        VkBool32 presentSupport = queryDevice.getSurfaceSupportKHR(i, surface);
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

bool Engine::checkDeviceExtensionSupport(vk::PhysicalDevice queryDevice) {
    // Use device wrapper
    std::vector<vk::ExtensionProperties> availableExtensions = queryDevice.enumerateDeviceExtensionProperties();

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

SwapChainSupportDetails Engine::querySwapChainSupport(vk::PhysicalDevice queryDevice) {
    SwapChainSupportDetails details;
    // Use device wrapper, pass C++ surface object
    details.capabilities = queryDevice.getSurfaceCapabilitiesKHR(surface);
    details.formats = queryDevice.getSurfaceFormatsKHR(surface);
    details.presentModes = queryDevice.getSurfacePresentModesKHR(surface);
    return details;
}

vk::SurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    // Check if formats are empty before accessing [0]
    if (availableFormats.empty()) {
        throw std::runtime_error("No suitable surface formats found!");
    }
    return availableFormats[0];
}

vk::PresentModeKHR Engine::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo; // Guaranteed available
}

vk::Extent2D Engine::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        // Get extent from Window object (returns VkExtent2D)
        int width, height;
        glfwGetFramebufferSize(window->getGLFWwindow(), &width, &height);
        VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        // Clamp extent
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        // Need to return vk::Extent2D
        return vk::Extent2D{actualExtent.width, actualExtent.height};
    }
}

void Engine::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{}; // Enable features as needed
    // Example: deviceFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo createInfo;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    // Use physical device wrapper
    device = physicalDevice.createDevice(createInfo);

    // Get queue handles using device wrapper
    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

void Engine::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndicesArray[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = nullptr; // Pass vk::SwapchainKHR() or nullptr

    // Use the device wrapper to create the swapchain
    swapChain = device.createSwapchainKHR(createInfo);

    // Retrieve images using the device wrapper
    swapChainImages = device.getSwapchainImagesKHR(swapChain);

    // Store format and extent
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void Engine::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vk::ImageViewCreateInfo createInfo;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = swapChainImageFormat;

        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;

        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        swapChainImageViews[i] = device.createImageView(createInfo);
    }
}

vk::ShaderModule Engine::createShaderModule(const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    return device.createShaderModule(createInfo);
}

void Engine::createRenderPass() {
    vk::AttachmentDescription colorAttachment;
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentDescription depthAttachment;
    depthAttachment.format = depthFormat;
    depthAttachment.samples = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    renderPass = device.createRenderPass(renderPassInfo);
}

void Engine::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboLayoutBinding;
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {uboLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
}

void Engine::createGraphicsPipeline() {
    auto vertShaderCode = readFile("shaders/shader.vert.spv");
    auto fragShaderCode = readFile("shaders/shader.frag.spv");

    vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
        {}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main"
    );
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
        {}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main"
    );
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
        {}, bindingDescription, attributeDescriptions // Use constructors
    );

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
        {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE
    );

    // Dynamic states for viewport and scissor
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, dynamicStates);

    // Viewport and Scissor state are set dynamically, so these are ignored
    vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);

    vk::PipelineRasterizationStateCreateInfo rasterizer(
        {}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise, // Adjust based on model winding
        VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
    );

    vk::PipelineMultisampleStateCreateInfo multisampling(
        {}, vk::SampleCountFlagBits::e1, VK_FALSE
    );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;
    // Other blend fields are ignored if blendEnable is false

    vk::PipelineColorBlendStateCreateInfo colorBlending(
        {}, VK_FALSE, vk::LogicOp::eCopy, 1, &colorBlendAttachment
    );

    vk::PipelineDepthStencilStateCreateInfo depthStencil;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, 1, &descriptorSetLayout);
    pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

    vk::GraphicsPipelineCreateInfo pipelineInfo(
        {}, // Flags
        shaderStages,
        &vertexInputInfo,
        &inputAssembly,
        nullptr, // pTessellationState
        &viewportState, // Viewport/Scissor set dynamically
        &rasterizer,
        &multisampling,
        &depthStencil,
        &colorBlending,
        &dynamicStateInfo, // Dynamic States
        pipelineLayout,
        renderPass,
        0 // subpass
    );

    // Use createGraphicsPipelineUnique for automatic cleanup potential
    auto result = device.createGraphicsPipeline(nullptr, pipelineInfo);
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create graphics pipeline! Error: " + vk::to_string(result.result));
    }
    graphicsPipeline = result.value;

    device.destroyShaderModule(fragShaderModule, nullptr);
    device.destroyShaderModule(vertShaderModule, nullptr);
}

void Engine::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<vk::ImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };

        vk::FramebufferCreateInfo framebufferInfo(
            {}, // flags
            renderPass,
            static_cast<uint32_t>(attachments.size()), // attachmentCount = 2
            attachments.data(), // pAttachments
            swapChainExtent.width,
            swapChainExtent.height,
            1 // layers
        );
        swapChainFramebuffers[i] = device.createFramebuffer(framebufferInfo);
    }
}

void Engine::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
    vk::CommandPoolCreateInfo poolInfo(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        queueFamilyIndices.graphicsFamily.value()
    );
    commandPool = device.createCommandPool(poolInfo);
}

uint32_t Engine::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

void Engine::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory) {
    vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
    buffer = device.createBuffer(bufferInfo);

    vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);
    vk::MemoryAllocateInfo allocInfo(memRequirements.size, findMemoryType(memRequirements.memoryTypeBits, properties));

    bufferMemory = device.allocateMemory(allocInfo);
    device.bindBufferMemory(buffer, bufferMemory, 0);
}

vk::CommandBuffer Engine::beginSingleTimeCommands() {
    vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);
    return commandBuffer;
}

void Engine::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    commandBuffer.end();

    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &commandBuffer, 0, nullptr); // Adjusted SubmitInfo
    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, commandBuffer);
}

void Engine::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
    vk::BufferCopy copyRegion(0, 0, size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
    endSingleTimeCommands(commandBuffer);
}

void Engine::createVertexBuffer() {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer, stagingBufferMemory);

    void* data = device.mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    device.unmapMemory(stagingBufferMemory);

    createBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    device.destroyBuffer(stagingBuffer);
    device.freeMemory(stagingBufferMemory);
}

void Engine::createIndexBuffer() {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer, stagingBufferMemory);

    void* data = device.mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, indices.data(), (size_t)bufferSize);
    device.unmapMemory(stagingBufferMemory);

    createBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    device.destroyBuffer(stagingBuffer);
    device.freeMemory(stagingBufferMemory);
}

void Engine::createUniformBuffers() {
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize,
                     vk::BufferUsageFlagBits::eUniformBuffer,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     uniformBuffers[i], uniformBuffersMemory[i]);
        uniformBuffersMapped[i] = device.mapMemory(uniformBuffersMemory[i], 0, bufferSize);
    }
}

void Engine::createDescriptorPool() {
    std::array<vk::DescriptorPoolSize, 1> poolSizes = {
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT))
    };
    vk::DescriptorPoolCreateInfo poolInfo({}, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), poolSizes);

    descriptorPool = device.createDescriptorPool(poolInfo);
}

void Engine::createDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, layouts);

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    descriptorSets = device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo(uniformBuffers[i], 0, sizeof(UniformBufferObject));
        vk::WriteDescriptorSet descriptorWrite(
            descriptorSets[i], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, bufferInfo, nullptr
        );
        device.updateDescriptorSets(descriptorWrite, nullptr); // Simplified call
    }
}

void Engine::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk::CommandBufferAllocateInfo allocInfo(
        commandPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(commandBuffers.size())
    );
    commandBuffers = device.allocateCommandBuffers(allocInfo);
}

void Engine::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        imageAvailableSemaphores[i] = device.createSemaphore(semaphoreInfo);
        renderFinishedSemaphores[i] = device.createSemaphore(semaphoreInfo);
        inFlightFences[i] = device.createFence(fenceInfo);
    }
}

void Engine::drawFrame() {
    // Wait for the previous frame to finish
    (void)device.waitForFences(inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire an image from the swap chain
    vk::ResultValue<uint32_t> acquireResult = device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], nullptr);

    if (acquireResult.result == vk::Result::eErrorOutOfDateKHR || acquireResult.result == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
        return;
    } else if (acquireResult.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }
    uint32_t imageIndex = acquireResult.value;

    // Reset the fence only if we are submitting work
    device.resetFences(inFlightFences[currentFrame]);

    // Record command buffer
    vk::CommandBuffer commandBuffer = commandBuffers[currentFrame];
    commandBuffer.reset();
    recordCommandBuffer(commandBuffer, imageIndex);
    updateUniformBuffer(currentFrame);

    // Submit the command buffer
    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);

    // Present the swap chain image
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vk::Result presentResult = presentQueue.presentKHR(presentInfo);

    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || framebufferResized) {
         framebufferResized = false;
         recreateSwapChain();
    } else if (presentResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Engine::updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), static_cast<float>(glfwGetTime()) * glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = camera.getViewMatrix();
    ubo.proj = camera.getProjectionMatrix(window->getAspectRatio());

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Engine::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo{};
    commandBuffer.begin(beginInfo);

    // Define clear values for BOTH color and depth
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0}; // Clear depth to 1.0 (far plane)

    vk::RenderPassBeginInfo renderPassInfo(
        renderPass, swapChainFramebuffers[imageIndex],
        vk::Rect2D({0, 0}, swapChainExtent),
        static_cast<uint32_t>(clearValues.size()), // clearValueCount = 2
        clearValues.data() // pClearValues
    );

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    // Set dynamic viewport and scissor
    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f);
    commandBuffer.setViewport(0, viewport);
    vk::Rect2D scissor({0, 0}, swapChainExtent);
    commandBuffer.setScissor(0, scissor);

    vk::Buffer vertexBuffers[] = {vertexBuffer};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets); // Simplified call
    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSets[currentFrame], nullptr); // Simplified call

    commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();
}

void Engine::cleanupSwapChain() {
    // Destroy depth resources that depend on swap chain size
    device.destroyImageView(depthImageView);
    device.destroyImage(depthImage);
    device.freeMemory(depthImageMemory);

    for (auto framebuffer : swapChainFramebuffers) {
        device.destroyFramebuffer(framebuffer);
    }
    device.destroyPipeline(graphicsPipeline);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyRenderPass(renderPass);
    for (auto imageView : swapChainImageViews) {
        device.destroyImageView(imageView);
    }
    device.destroySwapchainKHR(swapChain);
}

void Engine::cleanup() {
    cleanupSwapChain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (uniformBuffersMemory[i]) { // Check if memory was allocated
             device.unmapMemory(uniformBuffersMemory[i]);
        }
        if (uniformBuffers[i]) { // Check if buffer was created
             device.destroyBuffer(uniformBuffers[i]);
        }
        if (uniformBuffersMemory[i]) {
            device.freeMemory(uniformBuffersMemory[i]);
        }
    }

    device.destroyDescriptorPool(descriptorPool);
    device.destroyDescriptorSetLayout(descriptorSetLayout);

    device.destroyBuffer(indexBuffer);
    device.freeMemory(indexBufferMemory);
    device.destroyBuffer(vertexBuffer);
    device.freeMemory(vertexBufferMemory);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        device.destroySemaphore(renderFinishedSemaphores[i]);
        device.destroySemaphore(imageAvailableSemaphores[i]);
        device.destroyFence(inFlightFences[i]);
    }

    device.destroyCommandPool(commandPool);
    device.destroy(); // Destroy logical device

    if (enableValidationLayers && debugMessenger) {
        // Use C proxy function, pass instance, messenger, and nullptr allocator
        // Rely on implicit conversion from vk::DebugUtilsMessengerEXT to VkDebugUtilsMessengerEXT
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    if (surface) {
        instance.destroySurfaceKHR(surface);
    }
    if (instance) {
        instance.destroy(); // Destroy instance
    }

    // Window cleanup happens via unique_ptr
}

void Engine::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window->getGLFWwindow(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window->getGLFWwindow(), &width, &height);
        glfwWaitEvents();
    }

    device.waitIdle();
    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createDepthResources();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
}

// --- NEW Depth Format Helper Implementations ---

vk::Format Engine::findSupportedFormat(
    const std::vector<vk::Format>& candidates,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features)
{
    for (vk::Format format : candidates) {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

vk::Format Engine::findDepthFormat() {
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

void Engine::createDepthResources() {
    depthFormat = findDepthFormat();

    vk::ImageCreateInfo imageInfo(
        {}, // flags
        vk::ImageType::e2D,
        depthFormat,
        vk::Extent3D{swapChainExtent.width, swapChainExtent.height, 1},
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1, // samples (must match render pass)
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment, // usage
        vk::SharingMode::eExclusive,
        0, nullptr, // queueFamilyIndices (exclusive)
        vk::ImageLayout::eUndefined // initialLayout
    );

    depthImage = device.createImage(imageInfo);

    vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(depthImage);
    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size,
        findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    );

    depthImageMemory = device.allocateMemory(allocInfo);
    device.bindImageMemory(depthImage, depthImageMemory, 0);

    vk::ImageViewCreateInfo viewInfo(
        {}, // flags
        depthImage,
        vk::ImageViewType::e2D,
        depthFormat,
        {}, // components (default)
        vk::ImageSubresourceRange{
            vk::ImageAspectFlagBits::eDepth, // aspectMask
            0, // baseMipLevel
            1, // levelCount
            0, // baseArrayLayer
            1 // layerCount
        }
    );

    depthImageView = device.createImageView(viewInfo);

    // Note: Image layout transition is often handled implicitly by the render pass
    // or can be done explicitly here using begin/endSingleTimeCommands if needed.
}

} // namespace VulkanEngine
