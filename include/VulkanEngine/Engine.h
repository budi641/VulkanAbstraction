#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <vector>
#include <optional>
#include <string>
#include <array> // Added for attribute descriptions
#include <memory> // For unique_ptr

// Forward declarations for Vulkan handles to avoid including vulkan.h here
// This improves compile times if vulkan.h is large or changes often.
struct VkInstance_T; typedef VkInstance_T* VkInstance;
struct VkDebugUtilsMessengerEXT_T; typedef VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;
struct VkSurfaceKHR_T; typedef VkSurfaceKHR_T* VkSurfaceKHR;
struct VkPhysicalDevice_T; typedef VkPhysicalDevice_T* VkPhysicalDevice;
struct VkDevice_T; typedef VkDevice_T* VkDevice;
struct VkQueue_T; typedef VkQueue_T* VkQueue;
struct VkSwapchainKHR_T; typedef VkSwapchainKHR_T* VkSwapchainKHR;
struct VkImage_T; typedef VkImage_T* VkImage;
// Use typedefs for enums/structs as well for consistency, though less critical
typedef enum VkFormat VkFormat;
typedef struct VkExtent2D VkExtent2D;
struct VkImageView_T; typedef VkImageView_T* VkImageView;
struct VkRenderPass_T; typedef VkRenderPass_T* VkRenderPass;
struct VkDescriptorSetLayout_T; typedef VkDescriptorSetLayout_T* VkDescriptorSetLayout;
struct VkPipelineLayout_T; typedef VkPipelineLayout_T* VkPipelineLayout;
struct VkPipeline_T; typedef VkPipeline_T* VkPipeline;
struct VkFramebuffer_T; typedef VkFramebuffer_T* VkFramebuffer;
struct VkCommandPool_T; typedef VkCommandPool_T* VkCommandPool;
struct VkCommandBuffer_T; typedef VkCommandBuffer_T* VkCommandBuffer;
struct VkBuffer_T; typedef VkBuffer_T* VkBuffer;
struct VkDeviceMemory_T; typedef VkDeviceMemory_T* VkDeviceMemory;
struct VkDescriptorPool_T; typedef VkDescriptorPool_T* VkDescriptorPool;
struct VkDescriptorSet_T; typedef VkDescriptorSet_T* VkDescriptorSet;
struct VkSemaphore_T; typedef VkSemaphore_T* VkSemaphore;
struct VkFence_T; typedef VkFence_T* VkFence;
struct VkAllocationCallbacks;
struct VkDebugUtilsMessengerCreateInfoEXT;
typedef struct VkSurfaceCapabilitiesKHR VkSurfaceCapabilitiesKHR;
typedef struct VkSurfaceFormatKHR VkSurfaceFormatKHR;
typedef enum VkPresentModeKHR VkPresentModeKHR;
typedef struct VkVertexInputBindingDescription VkVertexInputBindingDescription;
typedef struct VkVertexInputAttributeDescription VkVertexInputAttributeDescription;
typedef enum VkBufferUsageFlagBits VkBufferUsageFlagBits;
typedef enum VkMemoryPropertyFlagBits VkMemoryPropertyFlagBits;
typedef uint32_t VkFlags;
typedef VkFlags VkBufferUsageFlags;
typedef VkFlags VkMemoryPropertyFlags;
typedef uint64_t VkDeviceSize;
typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT VkDebugUtilsMessageSeverityFlagBitsEXT;
typedef VkFlags VkDebugUtilsMessageTypeFlagsEXT;
struct VkDebugUtilsMessengerCallbackDataEXT;

// Include Window header
#include "VulkanEngine/Window.h"
#include "VulkanEngine/Camera.h" // Include Camera
#include "VulkanEngine/InputManager.h" // Include InputManager
#include "VulkanEngine/VulkanDevice.h" // Include VulkanDevice

namespace VulkanEngine {

// Structs moved from main.cpp (or potentially becoming classes later)
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    // texCoord will be added later if needed
    // glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class Engine {
public:
    // Constructor takes window parameters
    Engine(int width = 800, int height = 600, const std::string& title = "Vulkan Engine");
    ~Engine();

    // Delete copy constructor and assignment operator
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void run();

    Camera& getCamera() { return camera; } // Add getter for Camera
    const Camera& getCamera() const { return camera; }

    // Expose necessary getters if needed by InputManager callbacks (e.g., camera)
    // This might require passing 'this' (Engine*) to InputManager setup
    InputManager& getInputManager() { return *inputManager_; } // Might need this
    Window& getWindow() { return *window_; } // Might need this

private:
    // Removed initWindow(), now handled by Window class
    // void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    // Vulkan Core Initialization Steps (Removed redundant ones)
    // void createInstance();
    // void setupDebugMessenger();
    // void createSurface(); // Needs VkInstance from Engine
    // void pickPhysicalDevice();
    // void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createDepthResources();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();

    // Drawing and Frame Logic
    void drawFrame();
    void recreateSwapChain();
    void cleanupSwapChain();
    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentImage);

    // Vulkan Helpers (Removed more redundant ones)
    // REMOVED: querySwapChainSupport (moved to VulkanDevice)
    // REMOVED: chooseSwapSurfaceFormat (moved to VulkanDevice)
    // REMOVED: chooseSwapPresentMode (moved to VulkanDevice)
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities); // KEEP here
    vk::ShaderModule createShaderModule(const std::vector<char>& code); // Keep
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties); // Keep
    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory); // Keep
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size); // Keep
    // Command buffer helpers (keep)
    vk::CommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    // Input Handling
    void processInput(float deltaTime);
    void updateCameraVectors();
    // Input callbacks
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

    // Static Helpers
    std::vector<char> readFile(const std::string& filename);

    // --- Member Variables --- //
    std::unique_ptr<Window> window_; 
    std::unique_ptr<InputManager> inputManager_;
    Camera camera; 
    std::unique_ptr<VulkanDevice> vulkanDevice_;

    // Swap Chain
    vk::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::ImageView> swapChainImageViews;
    std::vector<vk::Framebuffer> swapChainFramebuffers;

    // Depth Buffer Resources (Keep)
    vk::Image depthImage = nullptr;
    vk::DeviceMemory depthImageMemory = nullptr;
    vk::ImageView depthImageView = nullptr;
    vk::Format depthFormat; // Keep, but populated via vulkanDevice_

    // Pipeline & Rendering (Keep)
    vk::RenderPass renderPass = nullptr;
    vk::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::PipelineLayout pipelineLayout = nullptr;
    vk::Pipeline graphicsPipeline = nullptr;
    vk::CommandPool commandPool = nullptr;

    // Buffers & Memory (Keep)
    vk::Buffer vertexBuffer = nullptr;
    vk::DeviceMemory vertexBufferMemory = nullptr;
    vk::Buffer indexBuffer = nullptr;
    vk::DeviceMemory indexBufferMemory = nullptr;
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped; // For UBO updates

    // Descriptors (Keep)
    vk::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::DescriptorSet> descriptorSets;

    // Command Buffers (one per frame in flight)
    std::vector<vk::CommandBuffer> commandBuffers;

    // Synchronization
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2; // Moved here
    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    uint32_t currentFrame = 0;

    bool framebufferResized = false; // Keep this!

    // Vertex Data (Keep for now)
    const std::vector<Vertex> vertices;
    const std::vector<uint16_t> indices;

    // Timing (Keep for now)
    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;

    // Removed Validation Layers definitions (now in VulkanDevice)
    // const std::vector<const char*> validationLayers = {...};
    // const std::vector<const char*> deviceExtensions = {...};
    // Removed enableValidationLayers toggle (passed to VulkanDevice)
    // #ifdef NDEBUG ... #else ... #endif

    // Removed old static members related to input/camera
};

} // namespace VulkanEngine 