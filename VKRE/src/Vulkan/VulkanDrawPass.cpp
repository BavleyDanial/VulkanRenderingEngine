#include "Renderer/Renderer.h"
#include <Vulkan/VulkanDrawPass.h>

namespace VKRE {

    VulkanDrawPass::VulkanDrawPass(VulkanResourceCache& cache, const VulkanGraphicsPipelineKey& key,
                                    VkDescriptorSet descriptorSet, VkShaderStageFlags pushConstantsShaderStages)
        :mCache(cache), mPipeline(cache.GetGraphicsPipeline(key)),
        mDescriptorSet(descriptorSet), mPushConstantsShaderStages(pushConstantsShaderStages) {}

    void VulkanDrawPass::Execute(VkCommandBuffer cmd, VkExtent2D extent, const RenderTargetInfo& targetInfo, VkDescriptorSet sceneSet) {
        if (!mPipeline || !mIsActive) return;

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
        if (sceneSet != VK_NULL_HANDLE)
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->layout, 1, 1, &sceneSet, 0, nullptr);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(extent.height);
        viewport.width = static_cast<float>(extent.width);
        viewport.height = -static_cast<float>(extent.height); 
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        VkBuffer lastIB = VK_NULL_HANDLE;
        for (const auto& draw : mDrawCommands) {
            DrawPushConstants pushConstants{};
            pushConstants.VertexBufferAddress = draw.VertexBufferAddress;
            pushConstants.Transform = draw.Transform;

            vkCmdPushConstants(cmd, mPipeline->layout, mPushConstantsShaderStages, 0, sizeof(DrawPushConstants), &pushConstants);

            VkBuffer ib = mCache.GetBufferData(draw.IndexBuffer)->buffer;
            if (ib != lastIB) {
                vkCmdBindIndexBuffer(cmd, ib, 0, VK_INDEX_TYPE_UINT32);
                lastIB = ib;
            }

            vkCmdDrawIndexed(cmd, draw.IndexCount, 1, draw.BaseIndex, draw.BaseVertex, 0);
        }

        vkCmdEndRendering(cmd);
        mDrawCommands.clear();
    }

    void VulkanDrawPass::SubmitDraw(const MeshDrawCommand& command) {
        mDrawCommands.push_back(command);
    }

    void VulkanDrawPass::ReBuild(VkDescriptorSet newDescriptorSet) {
        mDescriptorSet = newDescriptorSet;
    }

}
