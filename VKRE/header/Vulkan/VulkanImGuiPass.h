#pragma once

#include "VulkanContext.h"
#include "VulkanPresenter.h"
#include "VulkanTexture.h"

namespace VKRE {

    struct FrameInfo {
        uint32_t swapchainImageIdx = 0;
        VulkanTexture* drawImage = nullptr;
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
