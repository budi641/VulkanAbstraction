#include <vulkan/vulkan.hpp>

#include "VulkanEngine/Engine.h"
#include "VulkanEngine/Window.h"
#include "VulkanEngine/InputManager.h"
#include "VulkanEngine/Camera.h"
#include "VulkanEngine/VulkanDevice.h"

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

// --- REMOVED Global Proxy Function Definitions (Moved to VulkanDevice.cpp) ---

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

//-------------------------------------------------
// Engine Class Implementation
//-------------------------------------------------

Engine::Engine(int width, int height, const std::string& title)
    : window_(std::make_unique<Window>(width, height, title)),
      inputManager_(std::make_unique<InputManager>()),
      camera(glm::vec3(0.0f, 0.0f, 3.0f)),
// Determine validation layer setting based on build type
#ifdef NDEBUG
      vulkanDevice_(std::make_unique<VulkanDevice>(*window_, false)), // Release: validation off
#else
      vulkanDevice_(std::make_unique<VulkanDevice>(*window_, true)),  // Debug: validation on
#endif
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
    inputManager_->setupCallbacks(window_->getGLFWwindow(), this);
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
    if (inputManager_->isKeyPressed(GLFW_KEY_W))
        camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (inputManager_->isKeyPressed(GLFW_KEY_S))
        camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (inputManager_->isKeyPressed(GLFW_KEY_A))
        camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
    if (inputManager_->isKeyPressed(GLFW_KEY_D))
        camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);
     if (inputManager_->isKeyPressed(GLFW_KEY_SPACE))
        camera.processKeyboard(Camera_Movement::UP, deltaTime);
    if (inputManager_->isKeyPressed(GLFW_KEY_LEFT_CONTROL) || inputManager_->isKeyPressed(GLFW_KEY_RIGHT_CONTROL))
        camera.processKeyboard(Camera_Movement::DOWN, deltaTime);

    // Mouse movement (only when captured)
    if (inputManager_->isMouseCaptured()) {
        glm::vec2 mouseDelta = inputManager_->getMouseDelta();
        camera.processMouseMovement(mouseDelta.x, mouseDelta.y);
    }
}

// --- Main Loop ---
void Engine::mainLoop() {
    float lastFrameTime = 0.0f;

    while (!window_->shouldClose()) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents(); // Process events (triggers InputManager callbacks, updates currentX/Y)

        // SWAPPED ORDER:
        // 1. Use input state to update camera (calculates delta using currentX/Y and previous lastX/Y)
        this->processInput(deltaTime);
        // 2. Update InputManager state for the next frame (updates lastX/Y for next delta calc)
        inputManager_->processInput(deltaTime);

        // Handle resize (check flag set by Window callback)
        if (window_->wasResized()) {
            framebufferResized = true; // Signal drawFrame to handle recreate
            window_->resetResizedFlag();
        }

        drawFrame(); // Draw the frame (will handle recreate if framebufferResized is true)
    }
    vulkanDevice_->getDevice().waitIdle(); // Use device from VulkanDevice
}

void Engine::initVulkan() {
    // Instance, Debug Messenger, Surface, Physical Device, Logical Device
    // are now created within VulkanDevice constructor.

    // REMOVED: createInstance();
    // REMOVED: setupDebugMessenger();
    // REMOVED: createSurface();
    // REMOVED: pickPhysicalDevice();
    // REMOVED: createLogicalDevice();

    // Remaining setup using the created VulkanDevice
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createDepthResources(); // Create depth resources after command pool
    createFramebuffers();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

// --- Vulkan Implementation Details (Updated for vulkan.hpp) ---

void Engine::createSwapChain() {
    vk::PhysicalDevice physicalDevice = vulkanDevice_->getPhysicalDevice();
    vk::Device device = vulkanDevice_->getDevice();
    vk::SurfaceKHR surface = vulkanDevice_->getSurface();

    SwapChainSupportDetails swapChainSupport = vulkanDevice_->querySwapChainSupport();
    vk::SurfaceFormatKHR surfaceFormat = vulkanDevice_->chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = vulkanDevice_->chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = this->chooseSwapExtent(swapChainSupport.capabilities);

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

    QueueFamilyIndices indices = vulkanDevice_->getQueueFamilyIndices();
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
    vk::Device device = vulkanDevice_->getDevice();
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
    vk::Device device = vulkanDevice_->getDevice();
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    return device.createShaderModule(createInfo);
}

void Engine::createRenderPass() {
    vk::Device device = vulkanDevice_->getDevice();
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
    vk::Device device = vulkanDevice_->getDevice();
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
    vk::Device device = vulkanDevice_->getDevice();
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
    vk::Device device = vulkanDevice_->getDevice();
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
    vk::Device device = vulkanDevice_->getDevice();
    QueueFamilyIndices indices = vulkanDevice_->getQueueFamilyIndices();
    vk::CommandPoolCreateInfo poolInfo(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        indices.graphicsFamily.value()
    );
    commandPool = device.createCommandPool(poolInfo);
}

uint32_t Engine::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = vulkanDevice_->getPhysicalDevice().getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

void Engine::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory) {
    vk::Device device = vulkanDevice_->getDevice();
    vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
    buffer = device.createBuffer(bufferInfo);

    vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);
    vk::MemoryAllocateInfo allocInfo(memRequirements.size, findMemoryType(memRequirements.memoryTypeBits, properties));

    bufferMemory = device.allocateMemory(allocInfo);
    device.bindBufferMemory(buffer, bufferMemory, 0);
}

vk::CommandBuffer Engine::beginSingleTimeCommands() {
    vk::Device device = vulkanDevice_->getDevice();
    vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);
    return commandBuffer;
}

void Engine::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    vk::Device device = vulkanDevice_->getDevice();
    vk::Queue graphicsQueue = vulkanDevice_->getGraphicsQueue();
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

    void* data = vulkanDevice_->getDevice().mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vulkanDevice_->getDevice().unmapMemory(stagingBufferMemory);

    createBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vulkanDevice_->getDevice().destroyBuffer(stagingBuffer);
    vulkanDevice_->getDevice().freeMemory(stagingBufferMemory);
}

void Engine::createIndexBuffer() {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer, stagingBufferMemory);

    void* data = vulkanDevice_->getDevice().mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vulkanDevice_->getDevice().unmapMemory(stagingBufferMemory);

    createBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vulkanDevice_->getDevice().destroyBuffer(stagingBuffer);
    vulkanDevice_->getDevice().freeMemory(stagingBufferMemory);
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
        uniformBuffersMapped[i] = vulkanDevice_->getDevice().mapMemory(uniformBuffersMemory[i], 0, bufferSize);
    }
}

void Engine::createDescriptorPool() {
    vk::Device device = vulkanDevice_->getDevice();
    std::array<vk::DescriptorPoolSize, 1> poolSizes = {
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT))
    };
    vk::DescriptorPoolCreateInfo poolInfo({}, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), poolSizes);

    descriptorPool = device.createDescriptorPool(poolInfo);
}

void Engine::createDescriptorSets() {
    vk::Device device = vulkanDevice_->getDevice();
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
    vk::Device device = vulkanDevice_->getDevice();
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk::CommandBufferAllocateInfo allocInfo(
        commandPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(commandBuffers.size())
    );
    commandBuffers = device.allocateCommandBuffers(allocInfo);
}

void Engine::createSyncObjects() {
    vk::Device device = vulkanDevice_->getDevice();
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
    (void)vulkanDevice_->getDevice().waitForFences(inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire an image from the swap chain
    vk::ResultValue<uint32_t> acquireResult = vulkanDevice_->getDevice().acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], nullptr);

    if (acquireResult.result == vk::Result::eErrorOutOfDateKHR || acquireResult.result == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
        return;
    } else if (acquireResult.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }
    uint32_t imageIndex = acquireResult.value;

    // Reset the fence only if we are submitting work
    vulkanDevice_->getDevice().resetFences(inFlightFences[currentFrame]);

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

    vulkanDevice_->getGraphicsQueue().submit(submitInfo, inFlightFences[currentFrame]);

    // Present the swap chain image
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vk::Result presentResult = vulkanDevice_->getPresentQueue().presentKHR(presentInfo);

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
    ubo.proj = camera.getProjectionMatrix(window_->getAspectRatio());

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
    vulkanDevice_->getDevice().destroyImageView(depthImageView);
    vulkanDevice_->getDevice().destroyImage(depthImage);
    vulkanDevice_->getDevice().freeMemory(depthImageMemory);

    for (auto framebuffer : swapChainFramebuffers) {
        vulkanDevice_->getDevice().destroyFramebuffer(framebuffer);
    }
    vulkanDevice_->getDevice().destroyPipeline(graphicsPipeline);
    vulkanDevice_->getDevice().destroyPipelineLayout(pipelineLayout);
    vulkanDevice_->getDevice().destroyRenderPass(renderPass);
    for (auto imageView : swapChainImageViews) {
        vulkanDevice_->getDevice().destroyImageView(imageView);
    }
    vulkanDevice_->getDevice().destroySwapchainKHR(swapChain);
}

void Engine::cleanup() {
    cleanupSwapChain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (uniformBuffersMemory[i]) { // Check if memory was allocated
             vulkanDevice_->getDevice().unmapMemory(uniformBuffersMemory[i]);
        }
        if (uniformBuffers[i]) { // Check if buffer was created
             vulkanDevice_->getDevice().destroyBuffer(uniformBuffers[i]);
        }
        if (uniformBuffersMemory[i]) {
            vulkanDevice_->getDevice().freeMemory(uniformBuffersMemory[i]);
        }
    }

    // Check if pool/layout were created before destroying
    if (descriptorPool) {
        vulkanDevice_->getDevice().destroyDescriptorPool(descriptorPool);
    }
    if (descriptorSetLayout) {
        vulkanDevice_->getDevice().destroyDescriptorSetLayout(descriptorSetLayout);
    }

    // Check if buffers/memory were created before destroying
    if (indexBuffer) {
        vulkanDevice_->getDevice().destroyBuffer(indexBuffer);
    }
    if (indexBufferMemory) {
        vulkanDevice_->getDevice().freeMemory(indexBufferMemory);
    }
    if (vertexBuffer) {
        vulkanDevice_->getDevice().destroyBuffer(vertexBuffer);
    }
    if (vertexBufferMemory) {
        vulkanDevice_->getDevice().freeMemory(vertexBufferMemory);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // Check if sync objects were created before destroying
        if (renderFinishedSemaphores[i]) {
            vulkanDevice_->getDevice().destroySemaphore(renderFinishedSemaphores[i]);
        }
        if (imageAvailableSemaphores[i]) {
            vulkanDevice_->getDevice().destroySemaphore(imageAvailableSemaphores[i]);
        }
        if (inFlightFences[i]) {
            vulkanDevice_->getDevice().destroyFence(inFlightFences[i]);
        }
    }

    // Check if command pool was created before destroying
    if (commandPool) {
        vulkanDevice_->getDevice().destroyCommandPool(commandPool);
    }

    // REMOVED: Device, instance, surface, debug messenger cleanup.
    // These are now handled by ~VulkanDevice()

    // Window cleanup happens via unique_ptr
}

void Engine::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window_->getGLFWwindow(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window_->getGLFWwindow(), &width, &height);
        glfwWaitEvents();
    }

    vulkanDevice_->getDevice().waitIdle();
    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createDepthResources();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
}

void Engine::createDepthResources() {
    vk::Device device = vulkanDevice_->getDevice();
    // Use VulkanDevice's implementation
    depthFormat = vulkanDevice_->findDepthFormat(); 

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

// --- Swap Chain Helper Definitions (Keep chooseSwapExtent here) ---
vk::Extent2D Engine::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        // Use window_ member
        glfwGetFramebufferSize(window_->getGLFWwindow(), &width, &height);

        vk::Extent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
// --- End Swap Chain Helper Definitions ---

} // namespace VulkanEngine
