#pragma once

#include <Vulkan/VulkanResourceCache.h>
#include <Vulkan/VulkanDescriptors.h>

#include <glm/glm.hpp>
#include <vector>

namespace VKRE {

    struct RenderTargetInfo {
        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        VkRenderingAttachmentInfo* depthAttachment = nullptr;
        VkRenderingAttachmentInfo* stencilAttachment = nullptr;
    };

    struct MeshDrawCommand {
        GPUBufferHandle IndexBuffer;
        uint32_t IndexCount;
        uint32_t BaseIndex;
        uint32_t BaseVertex;
        uint64_t VertexBufferAddress;
        glm::mat4 Transform;
        glm::mat4 ViewProjection;
        int32_t TextureIndex = -1;
    };

    class VulkanDrawPass {
    public:
        VulkanDrawPass(VkDevice device, VulkanResourceCache& cache, const VulkanGraphicsPipelineKey& key, VkDescriptorSet descriptorSet,
                        VkShaderStageFlags pushConstantsShaderStages);

        void SetActive(bool enabled) { mIsActive = enabled; }
        bool IsActive() const { return mIsActive; }

        void SubmitDraw(const MeshDrawCommand& command);
        VkPipelineLayout GetPipelineLayout() const { return mPipeline->layout; }

        void ReBuild(VkDescriptorSet newDescriptorSet);
        void Execute(VkCommandBuffer cmd, VkExtent2D extent, const RenderTargetInfo& targetInfo,
                    VkDescriptorSet sceneSet, VkDescriptorSet bindlessSet);
    private:
        VkDevice mDevice;
        VulkanResourceCache& mCache;

        VulkanGraphicsPipeline* mPipeline;
        VkDescriptorSet mDescriptorSet;
        VkShaderStageFlags mPushConstantsShaderStages;
        std::vector<MeshDrawCommand> mDrawCommands;
        bool mIsActive = true;
    };

}
