#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>
#include <optional>

#include "VulkanEngine/Window.h" // Needs window for surface interaction

// Forward declare structs defined elsewhere (or move them here/to a common header)
namespace VulkanEngine {
struct QueueFamilyIndices;
}

namespace VulkanEngine {

// Keep QueueFamilyIndices definition ONLY here
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

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
    // Add getter for debug messenger
    vk::DebugUtilsMessengerEXT getDebugMessenger() const { return debugMessenger_; }

    // Expose necessary helper results
    // SwapChainSupportDetails querySwapChainSupport() const; // Might move this later
    // vk::Format findSupportedFormat(...) const; // Might move this later
    // uint32_t findMemoryType(...) const; // Might move this later

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();

    // Helpers moved from Engine
    bool checkValidationLayerSupport() const;
    std::vector<const char*> getRequiredExtensions() const;
    bool isDeviceSuitable(vk::PhysicalDevice physicalDevice) const;
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice) const;
    bool checkDeviceExtensionSupport(vk::PhysicalDevice physicalDevice) const;
    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;
    vk::Format findDepthFormat() const;

    // Keep debug callback static or global (C interface)
    // static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(...);

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