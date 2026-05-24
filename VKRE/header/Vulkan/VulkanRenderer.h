#pragma once

#include "ResourceManager/Resources.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"

#include "VulkanFrameManager.h"
#include "VulkanPresenter.h"

#include "VulkanDrawPass.h"
#include "VulkanComputePass.h"
#include "VulkanImGuiPass.h"

#include "VulkanResourceCache.h"
#include "VulkanDescriptors.h"
#include "VulkanImage.h"

#include "ResourceManager/ResourceManager.h"
#include "Events.h"

#include <concepts>
#include <limits>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

namespace VKRE {

    using ComputePassHandle = uint32_t;
    static constexpr ComputePassHandle INVALID_COMPUTE_PASS = std::numeric_limits<uint32_t>::max();

    using DrawPassHandle = uint32_t;
    static constexpr DrawPassHandle INVALID_DRAW_PASS = std::numeric_limits<uint32_t>::max();

    struct ComputePassDesc {
        std::string shaderPath;
        std::string debugName;
        std::vector<VkPushConstantRange> pushConstantRanges;
        uint32_t workgroupX = 16;
        uint32_t workgroupY = 16;
        uint32_t workgroupZ = 1;
    };

    struct DrawPassDesc {
        std::string shaderPath;
        std::string debugName;
        std::vector<VkPushConstantRange> pushConstantRanges;
        std::vector<VkFormat> colorAttachmentFormats;
        VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
        MeshHandle mesh;
    };

    class VulkanRenderer {
    public:
        VulkanRenderer(VulkanContext& context, ResourceManager& resourceManager);
        ~VulkanRenderer();

        template<typename Fn>
        requires std::invocable<Fn, VkCommandBuffer>
        void ImmediateSubmit(Fn&& fn) {
            VK_CHECK(vkResetFences(mContext.GetLogicalDevice().handle, 1, &mImmediateFence));
            vkResetCommandBuffer(mImmediateBuffer, 0);

            VkCommandBufferBeginInfo cmdBufferBeginInfo{};
            cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufferBeginInfo.pNext = nullptr;
            cmdBufferBeginInfo.pInheritanceInfo = nullptr;
            cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            VK_CHECK(vkBeginCommandBuffer(mImmediateBuffer, &cmdBufferBeginInfo));
            fn(mImmediateBuffer);
            VK_CHECK(vkEndCommandBuffer(mImmediateBuffer));

            VkCommandBufferSubmitInfo cmdSubmitInfo;
            cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            cmdSubmitInfo.pNext = nullptr;
            cmdSubmitInfo.commandBuffer = mImmediateBuffer;
            cmdSubmitInfo.deviceMask = 0;

            VkSubmitInfo2 info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
            info.pNext = nullptr;
            info.commandBufferInfoCount = 1;
            info.pCommandBufferInfos = &cmdSubmitInfo;

            VK_CHECK(vkQueueSubmit2(mContext.GetGraphicsQueue(), 1, &info, mImmediateFence));
            VK_CHECK(vkWaitForFences(mContext.GetLogicalDevice().handle, 1, &mImmediateFence, true, UINT64_MAX));
        }
        void ReSize(const WindowResizeEvent& event);

        void Render();
        void OnImGui();

        void UploadMesh(ResourceRef<MeshTag> mMesh, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
            VKRE::MeshHotData* hot = mResourceManager.GetMeshHot(mMesh.Get());

            mResourceCache->AllocateBuffer(hot->VertexBuffer);
            mResourceCache->AllocateBuffer(hot->IndexBuffer);

            mResourceCache->UploadBuffer(hot->VertexBuffer, vertices.data(), vertices.size() * sizeof(VKRE::Vertex), 0);
            mResourceCache->UploadBuffer(hot->IndexBuffer, indices.data(), indices.size() * sizeof(uint32_t), 0);
        }

        DrawPassHandle AddDrawPass(const DrawPassDesc& desc);
        void SetDrawPassData(DrawPassHandle handle, const void* data, uint32_t size);
        void ActivateDrawPass(DrawPassHandle handle) { mDrawPasses[handle].SetActive(true); }
        void DeActivateDrawPass(DrawPassHandle handle) { mDrawPasses[handle].SetActive(false); }

        ComputePassHandle AddComputePass(const ComputePassDesc& desc);
        void SetComputePassData(ComputePassHandle handle, const void* data, uint32_t size);
        void ActivateComputePass(ComputePassHandle handle) { mComputePasses[handle].SetActive(true); }
        void DeActivateComputePass(ComputePassHandle handle) { mComputePasses[handle].SetActive(false); }

        void ClearImage(VkCommandBuffer cmd);
    private:
        void CreateDrawImage();
        void ReCreateDrawImage();

        void CreateImmediateCommands();
        void InitPasses();
        void InitDescriptors();
        void InitDrawImageDescriptor();
    private:
        VulkanContext& mContext;
        ResourceManager& mResourceManager;

        std::unique_ptr<VulkanResourceCache> mResourceCache;
        VkCommandPool mImmediatePool;
        VkCommandBuffer mImmediateBuffer;
        VkFence mImmediateFence;

        std::unique_ptr<VulkanFrameManager> mFrameManager;
        std::unique_ptr<VulkanPresenter> mPresenter;
        std::unique_ptr<VulkanImage2D> mDrawImage; // TODO: Once done with managing deletion/creation internally turn into a value rather than a pointer

        std::vector<VulkanDrawPass> mDrawPasses;
        std::vector<VulkanComputePass> mComputePasses;
        std::unique_ptr<VulkanImGuiPass> mImGuiPass;

        DescriptorAllocator mGlobalDescriptorAllocator;
        VkDescriptorSet mDrawImageDescriptors;
        VkDescriptorSetLayout mDrawImageDescriptorLayout;

        VulkanUtils::DeletionQueue mDeletionQueue;
    };

}
