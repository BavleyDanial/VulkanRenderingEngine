#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace VKRE {

    class VulkanContext {
    public:
        VulkanContext();
        ~VulkanContext();


        VkInstance GetContext() const { return mInstance; }

    private:
        bool CheckValidationLayerSupport();

    private:
        VkInstance mInstance;

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
