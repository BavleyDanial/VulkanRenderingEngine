#pragma once
#include "VulkanRenderPass.h"

#include "VulkanPresenter.h"
#include "VulkanContext.h"

namespace VKRE {

    class VulkanImGuiPass : public IVulkanRenderPass {
    public:
        VulkanImGuiPass(VulkanContext& context, VulkanPresenter& presenter);
        ~VulkanImGuiPass();

        virtual void Execute(VkCommandBuffer cmd, const FrameInfo& frameInfo) override;
        virtual void OnImGui() override {}

    private:
        VulkanContext& mContext;
        VulkanPresenter& mPresenter;
        VkDescriptorPool mImGuiPool;
        VulkanUtils::DeletionQueue mDeletionQueue;
    };

}
