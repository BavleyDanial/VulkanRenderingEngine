#pragma once

#include "VulkanContext.h"
#include "VulkanPresenter.h"
#include "VulkanImage.h"

namespace VKRE {

    struct FrameInfo {
        uint32_t swapchainImageIdx = 0;
        VulkanImage2D* drawImage = nullptr;
    };

    class VulkanImGuiPass {
    public:
        VulkanImGuiPass(VulkanContext& context, VulkanPresenter& presenter);
        ~VulkanImGuiPass();

        void Execute(VkCommandBuffer cmd, const FrameInfo& frameInfo);

    private:
        VulkanContext& mContext;
        VulkanPresenter& mPresenter;
        VkDescriptorPool mImGuiPool;
    };

}
