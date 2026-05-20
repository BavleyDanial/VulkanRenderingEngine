#pragma once

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
        uint32_t vertexCount;
    };

    class VulkanRenderer {
    public:
        VulkanRenderer(VulkanContext& context, ResourceManager& resourceManager);
        ~VulkanRenderer();

        void Render();
        void OnImGui();
        void ReSize();

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

        void InitPasses();
        void InitDescriptors();
        void InitDrawImageDescriptor();
    private:
        VulkanContext& mContext;
        ResourceManager& mResourceManager;

        std::unique_ptr<VulkanResourceCache> mResourceCache;

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
