#pragma once

#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapChain.h"

#include <vector>

namespace VKRE {

    class VulkanContext {
    public:
        VulkanContext();
        ~VulkanContext();

        static const VkInstance GetInstance() { return sInstance; }
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
        bool CheckValidationLayerSupport();

    private:
        static inline VkInstance sInstance = VK_NULL_HANDLE;
        VkSurfaceKHR mSurface;

        VulkanPhysicalDevice mPhysicalDevice{};
        VulkanLogicalDevice mLogicalDevice{};

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
