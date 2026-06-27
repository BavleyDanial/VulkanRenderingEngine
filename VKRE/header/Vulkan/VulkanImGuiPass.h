#pragma once

#include <Vulkan/VulkanContext.h>

#include <Vulkan/VulkanImage.h>
#include <Vulkan/VulkanPresenter.h>

namespace VKRE {

    struct FrameInfo {
        uint32_t swapchainImageIdx = 0;
        VulkanImageData* drawImage = nullptr;
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
