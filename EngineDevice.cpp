#include "EngineDevice.h"

#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace aveng {

    // Local callback functions for validation layer
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,     // There are 4 levels of severity, you can compare them
        VkDebugUtilsMessageTypeFlagsEXT messageType,                // Another enum of types to help narrow down the incident
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,  // Refers to a struct full of useful debug information
        void *pUserData                                             // Contains a pointer, that was specified during setup of the callback. Allows us to pass data to it
    ) {
      std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

      // Always return false unless you're trying to test the validation layer itself.
      return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger
    ) {

      auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          instance,
          "vkCreateDebugUtilsMessengerEXT"
      );

        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

    }

    /*
     *  Free our Debug utility
     */
    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator
    ) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT"
        );

        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    /**
     * Class Functions
     */

    // Default Constructor - Initialize Vulkan
    EngineDevice::EngineDevice(AvengWindow &window) : window{window} {

        // Create the Vulkan instance
        createInstance();

        // Enable Debug validation layer -- Disable for Release Build
        setupDebugMessenger();

        // The surface is Vulkan's connection to our Window from GLFW.
        // This calls createWindowSurface from our _window class which is why
        // you will find it included in EnginDevice.hpp
        createSurface();

        // Choose your weapon (GPU), or multiple of them (super advanced)
        pickPhysicalDevice();

        // Determine thefeatures of our GPU we will be utilizing
        createLogicalDevice();

        // For command buffer allocation
        createCommandPool();
    }

    // Destructor
    EngineDevice::~EngineDevice() {
        vkDestroyCommandPool(device_, commandPool, nullptr);
        vkDestroyDevice(device_, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

          vkDestroySurfaceKHR(instance, surface_, nullptr);
          vkDestroyInstance(instance, nullptr);
    }

    void EngineDevice::createInstance() {

        // It's on the tin
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Zero App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "NonEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // [Validators] Create this instance's Debug Validation Layer
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;

        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

        hasGflwRequiredInstanceExtensions();

    }

    /**
     * Pick a GFX card in a series of steps
     * - Enumeration
     * - Evaluation - Make sure the device is suitable for Vulkan in its current context
     * Allows for ranking of most capable - See https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Physical_devices_and_queue_families - Base Device Suitability Checks
     */
    void EngineDevice::pickPhysicalDevice() {

        // Enumerate your gfx devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::cout << "Device count: " << deviceCount << std::endl;

        // Handle to our physical devices to be evaluated
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto &device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        std::cout << "physical device: " << properties.deviceName << std::endl;
    }

    /*
     *  Creation of a logical device.
     */
    void EngineDevice::createLogicalDevice() {

        // Collect our indicies; available Queues
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // For config
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

        // A complete config for each pipeline phase instance (don't quote me here)
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            // Create a new config
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            // Push it onto our configs
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Config - Device features
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        // Config - Core
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        // Enable features and extensions
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // This might not really be necessary anymore because
        // device specific validation layers have been deprecated
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        // Instantiate our logical device
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        // Get a queue handle for each queue family
        vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
        vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
    }

    void EngineDevice::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInfo.flags =
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    /**
    * Call to our _window class to create the window surface with GLFW
    */
    void EngineDevice::createSurface() { window.createWindowSurface(instance, &surface_); }

    /*
     * Determine if this card includes the necessary properties to support our application
     */
    bool EngineDevice::isDeviceSuitable(VkPhysicalDevice device) {

        // Look for available queue compatibilities within our device
        QueueFamilyIndices indices = findQueueFamilies(device);

        // Ensure required extensions are available
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        // Query for swapchain support so we can draw to our surface accordingly
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    /**
     *  Populate the struct which describes the details about the debug messenger.
     */
    void EngineDevice::populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT &createInfo
    ) {
      createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      // Bitwise OR enables all of the included
      createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      createInfo.pfnUserCallback = debugCallback;
      createInfo.pUserData = nullptr;  // Optional
    }

    void EngineDevice::setupDebugMessenger() {

        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;

        // Apply configurations to the struct
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    /**
     *  Check if all of the requested layers are available
     */
    bool EngineDevice::checkValidationLayerSupport() {

      uint32_t layerCount;

      // List all of the available layers.
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

      // Generate an array to store the names of our available validation layers in and acquire them
      std::vector<VkLayerProperties> availableLayers(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

      // Check and see if the validation layers we specified are supported
      for (const char *layerName : validationLayers) {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers) {
          if (strcmp(layerName, layerProperties.layerName) == 0) {
            layerFound = true;
            break;
          }
        }

        if (!layerFound) {
          return false;
        }
      }

      return true;
    }

    /**
     *  [Validators]
     *  Acquire a list of required extensions
     */
    std::vector<const char *> EngineDevice::getRequiredExtensions() {

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;

        // Collect a list of our required extensions
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // == "VK_EXT_debug_utils"
        }

        return extensions;
    }

    /**
    * [Validators]
    * Retrieve a list of available extensions on this system
    */
    void EngineDevice::hasGflwRequiredInstanceExtensions() {
      uint32_t extensionCount = 0;
      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

      // Hold the extension details
      std::vector<VkExtensionProperties> extensions(extensionCount);

      // Query the extension details
      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

      // Print to terminal
      std::cout << "available extensions:" << std::endl;
      std::unordered_set<std::string> available;
      for (const auto &extension : extensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
        available.insert(extension.extensionName);
      }

      // Print to terminal
      std::cout << "required extensions:" << std::endl;
      auto requiredExtensions = getRequiredExtensions();
      for (const auto &required : requiredExtensions) {
        std::cout << "\t" << required << std::endl;
        if (available.find(required) == available.end()) {
          throw std::runtime_error("Missing required glfw extension");
        }
      }
    }

    /*
    * [Validators]
    \*/
    bool EngineDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {

        uint32_t extensionCount;
        // Enumerate our devices extension properties
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        // Collect our device extensions now that we know how many we have
        vkEnumerateDeviceExtensionProperties(
            device,
            nullptr,
            &extensionCount,
            availableExtensions.data()
        );

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        // Remove names as we find them
        for (const auto &extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        // If all requried extensions were found, we should be left with no data
        return requiredExtensions.empty();
    }

    /**
    * Figure out the queue families supported by the device.
    */
    QueueFamilyIndices EngineDevice::findQueueFamilies(VkPhysicalDevice device) {

        QueueFamilyIndices indices;

        // Enumerate
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // Extract available queue families from our GPU driver(right?) as a vector of struct VkQueueFamilyProperties
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // By locating a queue for graphics and presentation we can ensure that our device is suitable
        int i = 0;
        for (const auto &queueFamily : queueFamilies) {

            // Find a graphics queue. We need at least 1 queue family that supports VK_QUEUE_GRAPHICS_BIT
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                indices.graphicsFamilyHasValue = true;
            }

            VkBool32 presentSupport = false;
            //  Look for a queue family that has the capability of presenting to our window surface
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);

            // Find a presentation queue. This could very well be the same thing as the graphics queue
            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
                indices.presentFamilyHasValue = true;
            }

            // Do we have a queue for each family of processes?
            if (indices.isComplete()) {
                break;
            }

            i++;
        }

      return indices;
    }

    /*
     * Validate the capabilities of our device for swapchain support.
     * Returns our swapchain details
     */
    SwapChainSupportDetails EngineDevice::querySwapChainSupport(VkPhysicalDevice device) {

        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface_,
                &presentModeCount,
                details.presentModes.data()
            );
        }

        return details;
    }

    VkFormat EngineDevice::findSupportedFormat(
        const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
      for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
          return format;
        } else if (
            tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
          return format;
        }
      }
      throw std::runtime_error("failed to find supported format!");
    }

    uint32_t EngineDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
      VkPhysicalDeviceMemoryProperties memProperties;
      vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
      for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
          return i;
        }
      }

      throw std::runtime_error("failed to find suitable memory type!");
    }

    void EngineDevice::createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory
    ) {
        // Create the Buffer given the provided information
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        // Query the buffer's memory requirements so we can allocate according to proper size & properties
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

        // 
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        // Bind the buffer to the memory we've allocated
        vkBindBufferMemory(device_, buffer, bufferMemory, 0);
    }

    VkCommandBuffer EngineDevice::beginSingleTimeCommands() {
      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandPool = commandPool;
      allocInfo.commandBufferCount = 1;

      VkCommandBuffer commandBuffer;
      vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      vkBeginCommandBuffer(commandBuffer, &beginInfo);
      return commandBuffer;
    }

    void EngineDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
      vkEndCommandBuffer(commandBuffer);

      VkSubmitInfo submitInfo{};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffer;

      vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
      vkQueueWaitIdle(graphicsQueue_);

      vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
    }

    void EngineDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
      VkCommandBuffer commandBuffer = beginSingleTimeCommands();

      VkBufferCopy copyRegion{};
      copyRegion.srcOffset = 0;  // Optional
      copyRegion.dstOffset = 0;  // Optional
      copyRegion.size = size;
      vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

      endSingleTimeCommands(commandBuffer);
    }

    void EngineDevice::copyBufferToImage(
        VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
      VkCommandBuffer commandBuffer = beginSingleTimeCommands();

      VkBufferImageCopy region{};
      region.bufferOffset = 0;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;

      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel = 0;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount = layerCount;

      region.imageOffset = {0, 0, 0};
      region.imageExtent = {width, height, 1};

      vkCmdCopyBufferToImage(
          commandBuffer,
          buffer,
          image,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          1,
          &region);
      endSingleTimeCommands(commandBuffer);
    }

    void EngineDevice::createImageWithInfo(
        const VkImageCreateInfo &imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory) {
      if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
      }

      VkMemoryRequirements memRequirements;
      vkGetImageMemoryRequirements(device_, image, &memRequirements);

      VkMemoryAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.allocationSize = memRequirements.size;
      allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

      if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
      }

      if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("failed to bind image memory!");
      }
    }

}  // namespace aveng