#include <Vulkan/VulkanDrawPass.h>
#include <cstring>
#include <vulkan/vulkan_core.h>

namespace VKRE {

    VulkanDrawPass::VulkanDrawPass(VulkanResourceCache& cache, const VulkanGraphicsPipelineKey& key, VkDescriptorSet descriptorSet, uint32_t vertexCount)
        :mPipeline(cache.GetGraphicsPipeline(key)), mDescriptorSet(descriptorSet), mVertexCount(vertexCount) {}

    void VulkanDrawPass::Execute(VkCommandBuffer cmd, VkExtent2D extent, const RenderTargetInfo& targetInfo) {
        if (!mPipeline) return;

        VkRenderingInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        info.renderArea = { { 0, 0 }, extent };
        info.layerCount = 1;
        info.colorAttachmentCount = static_cast<uint32_t>(targetInfo.colorAttachments.size());
        info.pColorAttachments = static_cast<uint32_t>(targetInfo.colorAttachments.size()) ? targetInfo.colorAttachments.data() : nullptr;
        info.pDepthAttachment = targetInfo.depthAttachment;
        info.pStencilAttachment = targetInfo.stencilAttachment;

        vkCmdBeginRendering(cmd, &info);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->pipeline);

        if (mDescriptorSet != VK_NULL_HANDLE)
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->layout, 0, 1, &mDescriptorSet, 0, nullptr);
        if (!mPushConstantData.empty())
            vkCmdPushConstants(cmd, mPipeline->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, mPushConstantData.size(), mPushConstantData.data());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdDraw(cmd, mVertexCount, 1, 0, 0);

        vkCmdEndRendering(cmd);
    }

    void VulkanDrawPass::SetPushConstantData(const void* data, uint32_t size) {
        mPushConstantData.resize(size);
        memcpy(mPushConstantData.data(), data, size);
    }

    void VulkanDrawPass::ReBuild(VkDescriptorSet newDescriptorSet) {
        mDescriptorSet = newDescriptorSet;
    }

}
