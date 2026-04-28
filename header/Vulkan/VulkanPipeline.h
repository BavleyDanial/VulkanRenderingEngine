#pragma once

#include <vulkan/vulkan.h>

namespace VKRE {

    struct VulkanPipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;

        bool Succeeded() const { return pipeline != VK_NULL_HANDLE && layout != VK_NULL_HANDLE; }

        void Destroy(VkDevice device) {
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, pipeline, nullptr);
            }
            if (layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(device, layout, nullptr);
            }
            pipeline = VK_NULL_HANDLE;
            layout = VK_NULL_HANDLE;
        }
    };

}
