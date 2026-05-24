#pragma once

#include <Vulkan/VulkanUtils.h>

#include <Vulkan/VulkanPhysicalDevice.h>
#include <Vulkan/VulkanLogicalDevice.h>

#include <Window/GlfwWindow.h>

#include <vector>
#include <memory>

namespace VKRE {

    class VulkanContext {
    public:
        VulkanContext(Window& window);
        ~VulkanContext();

        static const VkInstance GetInstance() { return sInstance; }
        VmaAllocator GetAllocator() { return mAllocator; }
        VkSurfaceKHR GetSurface() const { return mSurface; }


        const VulkanPhysicalDevice& GetPhysicalDevice() const { return mPhysicalDevice; }
        const VulkanLogicalDevice& GetLogicalDevice() const { return mLogicalDevice; }
        const QueueFamilyIndinces& GetQueueFamilies() const { return mPhysicalDevice.queueFamilyIndicies; }
        const VkQueue GetGraphicsQueue() const { return mLogicalDevice.graphicsQueue; }
        const VkQueue GetPresentQueue() const { return mLogicalDevice.presentQueue; }

        bool IsValidationLayersEnabled() const { return mEnableValidationLayers; }
        uint32_t GetValidationLayersCount() const { return static_cast<uint32_t>(mValidationLayers.size()); }
        std::vector<const char*> GetValidationLayers() const { return mValidationLayers; }

    private:
        static inline VkInstance sInstance = VK_NULL_HANDLE;
        std::shared_ptr<Window> mWindow;
        VkSurfaceKHR mSurface;

        VulkanPhysicalDevice mPhysicalDevice{};
        VulkanLogicalDevice mLogicalDevice{};

        VmaAllocator mAllocator;
        VulkanUtils::DeletionQueue mDeletionQueue;

        // TODO: Make validation layers only available in debug mode
        const std::vector<const char*> mValidationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        #ifdef NDEBUG // TODO: Add custom macro
        const bool mEnableValidationLayers = false;
        #else
        const bool mEnableValidationLayers = true;
        #endif
    };

}
