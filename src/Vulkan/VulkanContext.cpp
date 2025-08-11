#include <Vulkan/VulkanContext.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <Engine.h>
#include <GLFW/glfw3.h>

#include <cstring>

namespace VKRE {

    VulkanContext::VulkanContext(std::shared_ptr<Window> window) {
        if (mEnableValidationLayers && !VulkanUtils::CheckValidationLayerSupport(mValidationLayers)) {
            std::println("Failed to create Vulkan Instance: Validation Layers are not supported!");
            abort();
        }

        mWindow = window;

        if (!sInstance) {
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "VK Rendering Engine";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0 , 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

            // TODO: Check if all required extensions are available
            std::vector<const char*> extensions = window->GetWindowExtensions();
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

            VK_CHECK(vkCreateInstance(&createInfo, nullptr, &sInstance));
        }

        // TODO: Change this to be API agnostic
        GLFWwindow* glfwWindow = window->GetGLFWwindow();
        if (glfwCreateWindowSurface(sInstance, glfwWindow, nullptr, &mSurface) != VK_SUCCESS) {
            std::println("Failed to create Vulkan Surface!");
            abort();
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
            std::println("Failed to choose Vulkan Physical Device!");
            abort();
        }

        VulkanLogicalDeviceBuilder deviceBuilder(mPhysicalDevice);
        std::optional<VulkanLogicalDevice> logicalDevice = deviceBuilder.Build();
        if (logicalDevice.has_value()) {
            mLogicalDevice = logicalDevice.value();
        } else {
            std::println("Failed to choose Vulkan Physical Device!");
            abort();
        }

        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.instance = sInstance;
        allocatorInfo.device = logicalDevice->handle;
        allocatorInfo.physicalDevice = physicalDevice->handle;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        vmaCreateAllocator(&allocatorInfo, &mAllocator);
        mDeletionQueue.PushDeleteFunc([&]() { vmaDestroyAllocator(mAllocator); });
    }

    VulkanContext::~VulkanContext() {
        mDeletionQueue.Flush();
        mLogicalDevice.Destroy();
        vkDestroySurfaceKHR(sInstance, mSurface, nullptr);
        vkDestroyInstance(sInstance, nullptr);
    }
}
