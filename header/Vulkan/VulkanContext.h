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

        bool IsValidationLayersEnabled() const { return mEnableValidationLayers; }
        uint32_t GetValidationLayersCount() const { return mValidationLayers.size(); }
        std::vector<const char*> GetValidationLayers() { return mValidationLayers; }

    private:
        bool CheckValidationLayerSupport();

    private:
        VkInstance mInstance;
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
