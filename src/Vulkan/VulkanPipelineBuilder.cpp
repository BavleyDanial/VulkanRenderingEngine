#include <Vulkan/VulkanPipelineBuilder.h>
#include <print>

namespace VKRE {

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetPipelineLayout(VkPipelineLayout pipelineLayout) {
        mPipelineLayout = pipelineLayout;
        return *this;
    }

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetShaderModule(VkShaderModule shaderModule, const char* entrypoint) {
        mShaderModule = shaderModule;
        mEntrypoint = entrypoint;
        return *this;
    }

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetFlags(VkPipelineCreateFlags flags) {
        mFlags |= flags;
        return *this;
    }

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetNext(void* pNext) {
        mPNext = pNext;
        return *this;
    }

    VulkanComputePipeline VulkanComputePipelineBuilder::Build(VkDevice device, VkPipelineCache cache) {
        if (mShaderModule == VK_NULL_HANDLE) {
            std::println("VulkanComputePipelineBuilder::Build Tried to build with invalide Shader Module");
            return {};
        }

        VkPipelineShaderStageCreateInfo stageinfo{};
        stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageinfo.pNext = nullptr;
        stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stageinfo.module = mShaderModule;
        stageinfo.pName = mEntrypoint;

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = mPNext;
        pipelineInfo.flags = mFlags;
        pipelineInfo.layout = mPipelineLayout;
        pipelineInfo.stage = stageinfo;

        VkPipeline pipeline = VK_NULL_HANDLE;
        if (vkCreateComputePipelines(device, cache, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            std::println("VulkanComputePipelineBuilder::Build Failed to create Pipeline");
            vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);
            return {};
        }

        return { pipeline, mPipelineLayout };
    }

}
