#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <optional>
#include <string>
#include <array> // Added for attribute descriptions

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
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    // texCoord will be added later if needed
    // glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};


class Engine {
public:
    Engine(int width = 800, int height = 600, const std::string& title = "Vulkan Engine");
    ~Engine();

    // Delete copy constructor and assignment operator
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void run();

private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    // Vulkan Core Initialization Steps (will be extracted later)
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
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
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentImage);

    // Vulkan Helpers (will be extracted later)
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    // Command buffer helpers (potential extraction)
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    // Input Handling (will be extracted later)
    void processInput();
    void updateCameraVectors();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

    // Static Helpers (moved outside or into specific classes later)
    static std::vector<char> readFile(const std::string& filename);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    // --- Member Variables (similar to VulkanApplication) ---
    int windowWidth;
    int windowHeight;
    std::string windowTitle;
    GLFWwindow* window = nullptr; // Initialize to nullptr

    // Vulkan Core Objects
    VkInstance instance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    VkSurfaceKHR surface = nullptr;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = nullptr;
    VkQueue graphicsQueue = nullptr;
    VkQueue presentQueue = nullptr;

    // Swap Chain
    VkSwapchainKHR swapChain = nullptr;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    // Pipeline & Rendering
    VkRenderPass renderPass = nullptr;
    VkDescriptorSetLayout descriptorSetLayout = nullptr;
    VkPipelineLayout pipelineLayout = nullptr;
    VkPipeline graphicsPipeline = nullptr;
    VkCommandPool commandPool = nullptr;

    // Buffers & Memory
    VkBuffer vertexBuffer = nullptr;
    VkDeviceMemory vertexBufferMemory = nullptr;
    VkBuffer indexBuffer = nullptr;
    VkDeviceMemory indexBufferMemory = nullptr;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped; // For UBO updates

    // Descriptors
    VkDescriptorPool descriptorPool = nullptr;
    std::vector<VkDescriptorSet> descriptorSets;

    // Command Buffers (one per frame in flight)
    std::vector<VkCommandBuffer> commandBuffers;

    // Synchronization
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2; // Moved here
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    // Application State
    bool framebufferResized = false;

    // Vertex Data (Hardcoded for now, will be refactored)
    const std::vector<Vertex> vertices;
    const std::vector<uint16_t> indices;

    // Camera controls (will be moved to Camera class)
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

    // Keyboard state (will be moved to Input class)
    bool keys[1024] = {0};

    // Timing (for movement speed independence)
    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;

    // Validation Layers (Configuration)
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
};

} // namespace VulkanEngine 