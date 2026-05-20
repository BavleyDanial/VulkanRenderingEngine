#pragma once

#include "VulkanResourceCache.h"

#include <vector>

namespace VKRE {

    struct RenderTargetInfo {
        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        VkRenderingAttachmentInfo* depthAttachment = nullptr;
        VkRenderingAttachmentInfo* stencilAttachment = nullptr;
    };

    class VulkanDrawPass {
    public:
        VulkanDrawPass(VulkanResourceCache& cache, const VulkanGraphicsPipelineKey& key, VkDescriptorSet descriptorSet, uint32_t vertexCount);

        void SetPushConstantData(const void* data, uint32_t size);
        void SetActive(bool enabled) { mIsActive = enabled; }
        bool IsActive() const { return mIsActive; }

        void ReBuild(VkDescriptorSet newDescriptorSet);
        void Execute(VkCommandBuffer cmd, VkExtent2D extent, const RenderTargetInfo& targetInfo);
    private:
        VulkanGraphicsPipeline* mPipeline;
        VkDescriptorSet mDescriptorSet;
        std::vector<uint8_t> mPushConstantData;
        uint32_t mVertexCount;
        bool mIsActive = true;
    };

}
