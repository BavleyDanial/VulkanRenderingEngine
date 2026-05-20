#pragma once

#include "VulkanPipeline.h"

namespace VKRE {

    class VulkanComputePipelineBuilder {
    public:
        VulkanComputePipelineBuilder& SetPipelineLayout(VkPipelineLayout pipelineLayout);
        VulkanComputePipelineBuilder& SetShaderModule(VkShaderModule shaderModule, const char* entrypoint = "main");
        VulkanComputePipelineBuilder& SetFlags(VkPipelineCreateFlags flags);
        VulkanComputePipelineBuilder& SetNext(void* pNext);

        VulkanComputePipeline Build(VkDevice device, VkPipelineCache cache = VK_NULL_HANDLE);

    private:
        VkPipelineLayout mPipelineLayout;
        VkPipelineCreateFlags mFlags = 0;
        void* mPNext = nullptr;

        VkShaderModule mShaderModule = VK_NULL_HANDLE;
        const char* mEntrypoint = "";
    };

}
