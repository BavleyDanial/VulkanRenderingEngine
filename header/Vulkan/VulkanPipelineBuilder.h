#pragma once

#include "VulkanPipeline.h"

#include <span>
#include <vector>

namespace VKRE {

    class VulkanComputePipelineBuilder {
    public:
        VulkanComputePipelineBuilder& SetShaderModule(VkShaderModule shaderModule, const char* entrypoint = "main");
        VulkanComputePipelineBuilder& SetDescriptorSetLayouts(std::span<VkDescriptorSetLayout> layouts);
        VulkanComputePipelineBuilder& AddPushConstantRange(uint32_t offset, uint32_t size);
        VulkanComputePipelineBuilder& SetFlags(VkPipelineCreateFlags flags);
        VulkanComputePipelineBuilder& SetNext(void* pNext);

        VulkanPipeline Build(VkDevice device, VkPipelineCache cache = VK_NULL_HANDLE) const;

    private:
        VkShaderModule mShaderModule = VK_NULL_HANDLE;
        const char* mEntrypoint = "";
        void* mPNext = nullptr;
        VkPipelineCreateFlags mFlags = 0;

        std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
        std::vector<VkPushConstantRange> mPushConstantRanges;
    };

}
