#include <Vulkan/VulkanPipelineBuilder.h>
#include <print>

namespace VKRE {

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetShaderModule(VkShaderModule shaderModule, const char* entrypoint) {
        mShaderModule = shaderModule;
        mEntrypoint = entrypoint;
        return *this;
    }

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetDescriptorSetLayouts(std::span<VkDescriptorSetLayout> layouts) {
        mDescriptorSetLayouts.assign(layouts.begin(), layouts.end());
        return *this;
    }

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::AddPushConstantRange(uint32_t offset, uint32_t size) {
        mPushConstantRanges.push_back({ VK_SHADER_STAGE_COMPUTE_BIT, offset, size });
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

    VulkanPipeline VulkanComputePipelineBuilder::Build(VkDevice device, VkPipelineCache cache) const {
        if (mShaderModule == VK_NULL_HANDLE) {
            std::println("VulkanComputePipelineBuilder::Build Tried to build with invalide Shader Module");
            return {};
        }

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.pSetLayouts = mDescriptorSetLayouts.empty() ? nullptr : mDescriptorSetLayouts.data();
        layoutInfo.setLayoutCount = static_cast<uint32_t>(mDescriptorSetLayouts.size());
        layoutInfo.pPushConstantRanges = mPushConstantRanges.empty() ? nullptr : mPushConstantRanges.data();
        layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(mPushConstantRanges.size());

        VkPipelineLayout layout = VK_NULL_HANDLE;
        if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            std::println("VulkanComputePipelineBuilder::Build Failed to create Pipeline Layout");
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
        pipelineInfo.flags= mFlags;
        pipelineInfo.layout = layout;
        pipelineInfo.stage = stageinfo;

        VkPipeline pipeline = VK_NULL_HANDLE;
        if (vkCreateComputePipelines(device, cache, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            std::println("VulkanComputePipelineBuilder::Build Failed to create Pipeline");
            vkDestroyPipelineLayout(device, layout, nullptr);
            return {};
        }

        return { pipeline, layout };
    }

}
