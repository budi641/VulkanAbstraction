#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <set>

// Forward declarations
namespace VulkanEngine {
class Window; // Use forward declaration instead of include if possible
struct QueueFamilyIndices; // Forward declare if definition is elsewhere
}

namespace VulkanEngine {

// --- Structs moved/defined here --- 
struct QueueFamilyIndices { // Keep definition here
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// Moved from Engine.h
struct SwapChainSupportDetails { 
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};
// --- End Structs ---

class VulkanDevice {
public:
    // Constructor takes dependencies
    VulkanDevice(Window& window, bool enableValidationLayers);
    ~VulkanDevice();

    // Prevent copying
    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    // Corrected Getters
    vk::Instance getInstance() const { return instance_; }
    vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    vk::Device getDevice() const { return device_; }
    vk::Queue getGraphicsQueue() const { return graphicsQueue_; }
    vk::Queue getPresentQueue() const { return presentQueue_; }
    vk::SurfaceKHR getSurface() const { return surface_; }
    bool validationLayersEnabled() const { return enableValidationLayers_; }
    QueueFamilyIndices getQueueFamilyIndices() const { return queueFamilyIndices_; }
    vk::DebugUtilsMessengerEXT getDebugMessenger() const { return debugMessenger_; }
    vk::Format findDepthFormat() const;

    // --- Swap Chain Helpers (Moved from Engine) --- 
    SwapChainSupportDetails querySwapChainSupport() const; 
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const;
    // chooseSwapExtent remains in Engine as it needs window size

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();

    // Helpers moved from Engine or internal
    bool checkValidationLayerSupport() const;
    std::vector<const char*> getRequiredExtensions() const;
    bool isDeviceSuitable(vk::PhysicalDevice physicalDevice) const;
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice) const;
    bool checkDeviceExtensionSupport(vk::PhysicalDevice physicalDevice) const;
    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;
    // Overload for querySwapChainSupport taking device explicitly
    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice physicalDevice) const; 

    // Members
    Window& window_; // Reference to the window
    bool enableValidationLayers_; // Store config

    vk::Instance instance_ = nullptr;
    vk::DebugUtilsMessengerEXT debugMessenger_ = nullptr;
    vk::SurfaceKHR surface_ = nullptr;
    vk::PhysicalDevice physicalDevice_ = nullptr;
    vk::Device device_ = nullptr;
    vk::Queue graphicsQueue_ = nullptr;
    vk::Queue presentQueue_ = nullptr;
    QueueFamilyIndices queueFamilyIndices_; // Store the found indices

    // Keep layers/extensions definition here or pass via config
    const std::vector<const char*> validationLayers_ = {
        "VK_LAYER_KHRONOS_validation"
    };
     const std::vector<const char*> deviceExtensions_ = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
};

} // namespace VulkanEngine 