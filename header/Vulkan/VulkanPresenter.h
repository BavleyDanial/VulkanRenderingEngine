#pragma once

#include "VulkanSwapChain.h"
#include "VulkanContext.h"

#include <Window/GlfwWindow.h>

namespace VKRE {

    class VulkanPresenter {
    public:
        VulkanPresenter(std::shared_ptr<VulkanContext> context);
        ~VulkanPresenter();

        VulkanSwapChain& GetSwapChain() { return mSwapChain; }

        const std::vector<VkImage>& GetImages() const { return mSwapChainImages; }
        const std::vector<VkImageView>& GetImageViews() const { return mSwapChainImageViews; }

        VkSemaphore& GetRenderCompleteSemaphore(uint32_t index) { return mRenderCompleteSemaphores[index]; }
    
    private:
        std::shared_ptr<VulkanContext> mContext;
        VulkanSwapChain mSwapChain{};
        std::vector<VkImage> mSwapChainImages;
        std::vector<VkImageView> mSwapChainImageViews;
        std::vector<VkSemaphore> mRenderCompleteSemaphores;
    };

}
