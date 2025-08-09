#include <Vulkan/VulkanContext.h>

#include <Engine.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <cstring>

namespace VKRE {

    VulkanContext::VulkanContext() {
        if (mEnableValidationLayers && !CheckValidationLayerSupport()) {
            assert("Failed to create Vulkan Instance: Validation Layers are not supported!");
        }

        if (sInstance) {
            assert("Vulkan context already exists!");
            return;
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "VK Rendering Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0 , 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

        // TODO: Check if all required extensions are available
        std::vector<const char*> extensions = Engine::GetInstance().GetWindow()->GetWindowExtensions();
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        if (mEnableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
            createInfo.ppEnabledLayerNames = mValidationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &sInstance) != VK_SUCCESS) {
            assert("Failed to create Vulkan Instance!");
        }

        // TODO: Change this to be API agnostic
        GLFWwindow* glfwWindow = Engine::GetInstance().GetWindow()->GetGLFWwindow();
        if (glfwCreateWindowSurface(sInstance, glfwWindow, nullptr, &mSurface) != VK_SUCCESS) {
            assert("Failed to create Vulkan Surface!");
        }

        VulkanPhysicalDeviceSelector deviceSelector(sInstance, mSurface);
        std::optional<VulkanPhysicalDevice> physicalDevice = deviceSelector.SetName("Main Rendering Device")
                                                            .SetRequiredQueueFamilies({ VK_QUEUE_GRAPHICS_BIT })
                                                            .SetRequiredExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME })
                                                            .SetRequiredFeatures13({ .synchronization2 = true, .dynamicRendering = true })
                                                            .SetRequiredFeatures12({ .descriptorIndexing = true, .bufferDeviceAddress = true })
                                                            .Select();
        if (physicalDevice.has_value()) {
            mPhysicalDevice = physicalDevice.value();
        } else {
            assert("Failed to choose Vulkan Physical Device!");
        }

        VulkanLogicalDeviceBuilder deviceBuilder(mPhysicalDevice);
        std::optional<VulkanLogicalDevice> logicalDevice = deviceBuilder.Build();
        if (logicalDevice.has_value()) {
            mLogicalDevice = logicalDevice.value();
        } else {
            assert("Failed to choose Vulkan Physical Device!");
        }

    }

    VulkanContext::~VulkanContext() {
        mLogicalDevice.Destroy();

        vkDestroySurfaceKHR(sInstance, mSurface, nullptr);
        vkDestroyInstance(sInstance, nullptr);
    }

    bool VulkanContext::CheckValidationLayerSupport() {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : mValidationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
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

}
