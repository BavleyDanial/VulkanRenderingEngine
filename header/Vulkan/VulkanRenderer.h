#pragma once

#include "VulkanUtils.h"

#include "VulkanContext.h"
#include "VulkanFrameManager.h"
#include "VulkanPresenter.h"
#include "VulkanImage.h"
#include "VulkanDescriptors.h"

#include <memory>

namespace VKRE {

    class VulkanRenderer {
    public:
        VulkanRenderer(std::shared_ptr<VulkanContext> context);
        ~VulkanRenderer();

        void Render();
        void DrawGradientBackground(VkCommandBuffer cmd);
        void ClearImage(VkCommandBuffer cmd);

    private:
        void CreateDrawImage();
        void InitDescriptors();

        // TODO: Abstract these to a pipeline file or leave them but utilise a pipeline file
        void InitPipelines();
        void InitBackgroundPipelines();
    private:
        std::shared_ptr<VulkanContext> mContext;
        std::unique_ptr<VulkanFrameManager> mFrameManager;
        std::unique_ptr<VulkanPresenter> mPresenter;
        std::unique_ptr<VulkanImage2D> mDrawImage;

        DescriptorAllocator mGlobalDescriptorAllocator;
        VkDescriptorSet mDrawImageDescriptors;
        VkDescriptorSetLayout mDrawImageDescriptorLayout;

        // TODO: Abstract these to a pipeline file
        VkPipeline mGradientPipeline;
        VkPipelineLayout mGradientPipelineLayout;

        VulkanUtils::DeletionQueue mDeletionQueue;
    };

}
