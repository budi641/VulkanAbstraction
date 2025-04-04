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

namespace VulkanEngine {

// Structs moved from main.cpp (or potentially becoming classes later)
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const { // Make const
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

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
    InputManager& getInputManager() { return *inputManager; } // Might need this
    Window& getWindow() { return *window; } // Might need this

private:
    // Removed initWindow(), now handled by Window class
    // void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    // Vulkan Core Initialization Steps
    void createInstance();
    void setupDebugMessenger();
    void createSurface(); // Needs VkInstance from Engine
    void pickPhysicalDevice();
    void createLogicalDevice();
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

    // Vulkan Helpers
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(vk::PhysicalDevice device);
    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    vk::ShaderModule createShaderModule(const std::vector<char>& code);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
    // Command buffer helpers (potential extraction)
    vk::CommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    // Input Handling (Still here for now, depends on Window)
    void processInput(float deltaTime); // New method for input handling
    void updateCameraVectors();
    // Removed framebufferResizeCallback, handled by Window class
    // static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    // Input callbacks remain for now, but need adjustment
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

    // Static Helpers
    std::vector<char> readFile(const std::string& filename);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    // --- Member Variables --- //
    std::string applicationName; // Added back for instance creation
    std::unique_ptr<Window> window; // Use unique_ptr for ownership
    // Removed windowWidth, windowHeight, windowTitle (now in Window), framebufferResized

    // Vulkan Core Objects (Keep these)
    vk::Instance instance = nullptr;
    vk::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::SurfaceKHR surface = nullptr;
    vk::PhysicalDevice physicalDevice = nullptr;
    vk::Device device = nullptr;
    vk::Queue graphicsQueue = nullptr;
    vk::Queue presentQueue = nullptr;

    // Swap Chain (Keep these)
    vk::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::ImageView> swapChainImageViews;
    std::vector<vk::Framebuffer> swapChainFramebuffers;

    // Depth Buffer Resources (NEW)
    vk::Image depthImage = nullptr;
    vk::DeviceMemory depthImageMemory = nullptr;
    vk::ImageView depthImageView = nullptr;
    vk::Format depthFormat;

    // Pipeline & Rendering
    vk::RenderPass renderPass = nullptr;
    vk::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::PipelineLayout pipelineLayout = nullptr;
    vk::Pipeline graphicsPipeline = nullptr;
    vk::CommandPool commandPool = nullptr;

    // Buffers & Memory
    vk::Buffer vertexBuffer = nullptr;
    vk::DeviceMemory vertexBufferMemory = nullptr;
    vk::Buffer indexBuffer = nullptr;
    vk::DeviceMemory indexBufferMemory = nullptr;
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped; // For UBO updates

    // Descriptors
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

    bool framebufferResized = false; // Add this back!

    // Vertex Data (Keep for now)
    const std::vector<Vertex> vertices;
    const std::vector<uint16_t> indices;

    // Camera controls (Keep for now)
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float cameraSpeed = 2.5f; // Adjusted speed

    // Mouse camera control (will be moved to Camera/Input class)
    bool firstMouse = true; // Prevent jump on first capture
    bool mouseCaptured = false;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = 0.0f; // Initialized in initWindow
    float lastY = 0.0f; // Initialized in initWindow
    float mouseSensitivity = 0.1f;

    // Keyboard state (Keep for now)
    bool keys[1024] = {0};

    // Timing (Keep for now)
    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;

    // Validation Layers (Keep)
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif

    // --- Removed Input Members ---
    // static std::array<bool, 1024> keys;
    // static std::array<bool, 8> mouseButtons;
    // static double lastX;
    // static double lastY;
    // static bool firstMouse;
    // static bool mouseCaptured;
    // static Camera* staticCameraPtr; // Removed static camera pointer

    // --- Removed Static Callbacks ---
    // static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    // static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    // static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    // static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    // --- Members ---
    std::unique_ptr<InputManager> inputManager; // Input Manager instance
    Camera camera; // Camera instance

    // Helper functions
    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    vk::Format findDepthFormat(); // NEW: Helper for depth format
};

} // namespace VulkanEngine 