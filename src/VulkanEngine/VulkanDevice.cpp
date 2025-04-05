#include "VulkanEngine/VulkanDevice.h"
#include "VulkanEngine/Window.h" // Include Window header

// Include libraries used in implementation
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <set>
#include <string>
#include <cstring> // For strcmp

namespace VulkanEngine {

// External C function pointers for debug messenger (defined in Engine.cpp or elsewhere)
extern VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
extern void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

// Helper function (can remain static or become a member)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}


VulkanDevice::VulkanDevice(Window& window, bool enableValidationLayers)
    : window_(window), enableValidationLayers_(enableValidationLayers)
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
}

VulkanDevice::~VulkanDevice() {
    if (device_) device_.destroy();
    if (enableValidationLayers_ && debugMessenger_) {
        // Use C proxy function, rely on implicit conversion for messenger
        DestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
    }
    if (surface_) instance_.destroySurfaceKHR(surface_);
    if (instance_) instance_.destroy();
}

void VulkanDevice::createInstance() {
    if (enableValidationLayers_ && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo(
        window_.getTitle().c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        "VulkanEngine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_3
    );

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo; // Use C++ type
    if (enableValidationLayers_) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers_.size());
        createInfo.ppEnabledLayerNames = validationLayers_.data();

        // Populate C++ struct
        debugCreateInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
        debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        // Cast the static C callback to the C++ function pointer type
        debugCreateInfo.pfnUserCallback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback);
        debugCreateInfo.pUserData = nullptr;

        createInfo.pNext = &debugCreateInfo; // Assign address to void*
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pNext = nullptr;
    }

    instance_ = vk::createInstance(createInfo);
}

void VulkanDevice::setupDebugMessenger() {
    if (!enableValidationLayers_) return;

    // Populate the C struct needed by the proxy function
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback; // Use the static C callback directly
    createInfo.pUserData = nullptr;

    // Call the C proxy function
    if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT*>(&debugMessenger_)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void VulkanDevice::createSurface() {
    // Delegate surface creation to the window object
    // Window::createWindowSurface expects vk::Instance but takes VkSurfaceKHR* output
    window_.createWindowSurface(instance_, reinterpret_cast<VkSurfaceKHR*>(&surface_));
}

void VulkanDevice::pickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> devices = instance_.enumeratePhysicalDevices();
    if (devices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice_ = device;
            // Optional: Prefer discrete GPU
            vk::PhysicalDeviceProperties properties = physicalDevice_.getProperties();
             if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                 std::cout << "Picked Discrete GPU: " << properties.deviceName << std::endl;
                 return; // Found discrete, use it
             }
        }
    }
     // If no discrete GPU found, physicalDevice_ should hold the first suitable one
    if (physicalDevice_){
         vk::PhysicalDeviceProperties properties = physicalDevice_.getProperties();
         std::cout << "Picked GPU: " << properties.deviceName << std::endl;
    } else {
         throw std::runtime_error("Failed to find a suitable GPU!");
    }

}

void VulkanDevice::createLogicalDevice() {
    queueFamilyIndices_ = findQueueFamilies(physicalDevice_); // Store indices

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        queueFamilyIndices_.graphicsFamily.value(),
        queueFamilyIndices_.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamilyIndex : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{}; // Enable features as needed

    vk::DeviceCreateInfo createInfo;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions_.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions_.data();

    if (enableValidationLayers_) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers_.size());
        createInfo.ppEnabledLayerNames = validationLayers_.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    device_ = physicalDevice_.createDevice(createInfo);

    graphicsQueue_ = device_.getQueue(queueFamilyIndices_.graphicsFamily.value(), 0);
    presentQueue_ = device_.getQueue(queueFamilyIndices_.presentFamily.value(), 0);
}

// --- Helper Implementations ---

bool VulkanDevice::checkValidationLayerSupport() const {
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : validationLayers_) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) return false;
    }
    return true;
}

std::vector<const char*> VulkanDevice::getRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers_) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

bool VulkanDevice::isDeviceSuitable(vk::PhysicalDevice queryDevice) const {
    QueueFamilyIndices indices = findQueueFamilies(queryDevice);
    bool extensionsSupported = checkDeviceExtensionSupport(queryDevice);
    // Swap chain support check might move to a dedicated SwapChain class later
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        // Need surface to check swap chain details
        // Temporarily query here, ideally surface is passed or handled differently
         if (!surface_) { 
             // Surface might not be created yet if called during picking
             // This indicates a potential ordering issue or need for refactoring
             // For now, assume it *will* be suitable if extensions ok
             // Or throw an error / handle it more gracefully
             // Let's query the surface formats/modes directly here for simplicity
              std::vector<vk::SurfaceFormatKHR> formats = queryDevice.getSurfaceFormatsKHR(surface_);
              std::vector<vk::PresentModeKHR> presentModes = queryDevice.getSurfacePresentModesKHR(surface_);
              swapChainAdequate = !formats.empty() && !presentModes.empty();
         } else {
              std::vector<vk::SurfaceFormatKHR> formats = queryDevice.getSurfaceFormatsKHR(surface_);
              std::vector<vk::PresentModeKHR> presentModes = queryDevice.getSurfacePresentModesKHR(surface_);
              swapChainAdequate = !formats.empty() && !presentModes.empty();
         }
    }
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(vk::PhysicalDevice queryDevice) const {
    QueueFamilyIndices indices;
    std::vector<vk::QueueFamilyProperties> queueFamilies = queryDevice.getQueueFamilyProperties();
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }
        // Need surface_ member to check presentation support
        if (surface_) {
            VkBool32 presentSupport = queryDevice.getSurfaceSupportKHR(i, surface_);
            if (presentSupport) {
                indices.presentFamily = i;
            }
        } // Handle case where surface might not exist yet if needed

        if (indices.isComplete()) break;
        i++;
    }
    return indices;
}

bool VulkanDevice::checkDeviceExtensionSupport(vk::PhysicalDevice queryDevice) const {
    std::vector<vk::ExtensionProperties> availableExtensions = queryDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(deviceExtensions_.begin(), deviceExtensions_.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

// Implementation for findSupportedFormat
vk::Format VulkanDevice::findSupportedFormat(
    const std::vector<vk::Format>& candidates,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features) const
{
    // Use member physicalDevice_
    for (vk::Format format : candidates) {
        vk::FormatProperties props = physicalDevice_.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

// Implementation for findDepthFormat
vk::Format VulkanDevice::findDepthFormat() const {
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

} // namespace VulkanEngine 