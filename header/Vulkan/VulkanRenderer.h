#pragma once

#include "VulkanUtils.h"
#include "VulkanContext.h"

#include "VulkanFrameManager.h"
#include "VulkanPresenter.h"
#include "VulkanRenderPass.h"

#include "VulkanResourceCache.h"
#include "VulkanDescriptors.h"
#include "VulkanPipeline.h"
#include "VulkanImage.h"

#include "ResourceManager/ResourceManager.h"

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
            VulkanPipeline compute;
            ComputePushConstants data;
        };

        std::vector<ComputeEffect> backgroundEffects;
        int currentBackgroundEffect = 0;

    public:
        VulkanRenderer(VulkanContext& context, ResourceManager& resourceManager);
        ~VulkanRenderer();

        void Render();
        void OnImGui();
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
        ResourceManager& mResourceManager;

        std::unique_ptr<VulkanResourceCache> mResourceCache;
        std::vector<ShaderHandle> mOwnedShaders; // TODO: Move this outside of renderer so that the shader ownership is from somewhere else not here

        std::unique_ptr<VulkanFrameManager> mFrameManager;
        std::unique_ptr<VulkanPresenter> mPresenter;
        std::unique_ptr<VulkanImage2D> mDrawImage; // TODO: Once done with managing deletion/creation internally turn into a value rather than a pointer

        std::vector<std::unique_ptr<IVulkanRenderPass>> mPasses;
        std::unique_ptr<IVulkanRenderPass> mImGuiPass;

        DescriptorAllocator mGlobalDescriptorAllocator;
        VkDescriptorSet mDrawImageDescriptors;
        VkDescriptorSetLayout mDrawImageDescriptorLayout;

        VulkanPipeline mComputePipeline; // TODO: Remove this later
        VulkanUtils::DeletionQueue mDeletionQueue;
    };

}
