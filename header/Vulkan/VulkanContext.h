#pragma once

#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"

#include <vulkan/vulkan_core.h>
#include <vector>

namespace VKRE {

    class VulkanContext {
    public:
        VulkanContext();
        ~VulkanContext();

        VkInstance GetInstance() const { return mInstance; }
        VkSurfaceKHR GetSurface() const { return mSurface; }

        bool IsValidationLayersEnabled() const { return mEnableValidationLayers; }
        uint32_t GetValidationLayersCount() const { return static_cast<uint32_t>(mValidationLayers.size()); }
        std::vector<const char*> GetValidationLayers() const { return mValidationLayers; }

    private:
        bool CheckValidationLayerSupport();

    private:
        VkInstance mInstance;
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
