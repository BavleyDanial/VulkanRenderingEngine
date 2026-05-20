#pragma once

#include "VulkanUtils.h"
#include "VulkanContext.h"

#include "VulkanFrameManager.h"
#include "VulkanPresenter.h"
#include "VulkanImGuiPass.h"
#include "VulkanComputePass.h"

#include "VulkanResourceCache.h"
#include "VulkanDescriptors.h"
#include "VulkanPipeline.h"
#include "VulkanImage.h"

#include "ResourceManager/ResourceManager.h"

#include <limits>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

namespace VKRE {

    using ComputePassHandle = uint32_t;
    static constexpr ComputePassHandle INVALID_COMPUTE_PASS = std::numeric_limits<uint32_t>::max();

    struct ComputePassDesc {
        std::string shaderPath;
        std::string debugName;
        std::vector<VkPushConstantRange> pushConstantRanges;
        uint32_t workgroupX = 16;
        uint32_t workgroupY = 16;
        uint32_t workgroupZ = 1;
    };

    class VulkanRenderer {
    public:
        VulkanRenderer(VulkanContext& context, ResourceManager& resourceManager);
        ~VulkanRenderer();

        void Render();
        void OnImGui();
        void ReSize();

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

        std::unique_ptr<VulkanImGuiPass> mImGuiPass;
        std::vector<VulkanComputePass> mComputePasses;

        DescriptorAllocator mGlobalDescriptorAllocator;
        VkDescriptorSet mDrawImageDescriptors;
        VkDescriptorSetLayout mDrawImageDescriptorLayout;

        VulkanUtils::DeletionQueue mDeletionQueue;
    };

}
