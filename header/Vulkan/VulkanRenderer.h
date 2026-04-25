#pragma once

#include "VulkanRenderPass.h"
#include "VulkanUtils.h"

#include "VulkanContext.h"
#include "VulkanFrameManager.h"
#include "VulkanPresenter.h"
#include "VulkanImage.h"
#include "VulkanDescriptors.h"

#include <memory>
#include <vector>

#include <glm/glm.hpp>

namespace VKRE {

    class VulkanRenderer {
    public:
        // NOTE: THIS IS JUST FOR ILLUSTRATION OF PUSH CONSTANTS, WILL BE REMOVED
        struct ComputePushConstants {
            glm::vec4 data1;
            glm::vec4 data2;
            glm::vec4 data3;
            glm::vec4 data4;
        };

        // NOTE: THIS IS JUST FOR ILLUSTRATION OF ImGui, WILL BE REMOVED
        struct ComputeEffect {
            const char* name;

            VkPipeline pipeline;
            VkPipelineLayout layout;

            ComputePushConstants data;
        };

        std::vector<ComputeEffect> backgroundEffects;
        int currentBackgroundEffect = 0;

    public:
        VulkanRenderer(VulkanContext& context);
        ~VulkanRenderer();

        void Render();
        void ReSize();
        void DrawGradientBackground(VkCommandBuffer cmd);
        void ClearImage(VkCommandBuffer cmd);

    private:
        void CreateDrawImage();
        void ReCreateDrawImage();

        void InitPasses();
        void InitDescriptors();
        void InitDrawImageDescriptor();

        // TODO: Abstract these to a pipeline file or leave them but utilise a pipeline file
        void InitPipelines();
        void InitBackgroundPipelines();
    private:
        VulkanContext& mContext;
        std::unique_ptr<VulkanFrameManager> mFrameManager;
        std::unique_ptr<VulkanPresenter> mPresenter;
        std::unique_ptr<VulkanImage2D> mDrawImage; // TODO: Once done with managing deletion/creation internally turn into a value rather than a pointer

        std::vector<std::unique_ptr<IVulkanRenderPass>> mPasses;

        DescriptorAllocator mGlobalDescriptorAllocator;
        VkDescriptorSet mDrawImageDescriptors;
        VkDescriptorSetLayout mDrawImageDescriptorLayout;

        // TODO: Abstract these to a pipeline file
        VkPipeline mGradientPipeline;
        VkPipelineLayout mGradientPipelineLayout;

        VulkanUtils::DeletionQueue mDeletionQueue;
    };

}
