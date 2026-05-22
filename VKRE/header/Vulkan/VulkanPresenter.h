#pragma once

#include "VulkanSwapChain.h"
#include "VulkanContext.h"

#include <Window/GlfwWindow.h>

namespace VKRE {

    class VulkanPresenter {
    public:
        VulkanPresenter(VulkanContext& context);
        ~VulkanPresenter();

        void ResizeSwapChain(uint32_t width, uint32_t height);
        VulkanSwapChain& GetSwapChain() { return mSwapChain; }

        const std::vector<VkImage>& GetImages() const { return mSwapChainImages; }
        const std::vector<VkImageView>& GetImageViews() const { return mSwapChainImageViews; }

        VkSemaphore& GetRenderCompleteSemaphore(uint32_t index) { return mRenderCompleteSemaphores[index]; }

    private:
        void CreateSwapChain(uint32_t width, uint32_t height);
        void DestroySwapChain();

        void GetSwapChainImages();
        void GetSwapChainImageViews();

    private:
        VulkanContext& mContext;
        VulkanSwapChain mSwapChain{};
        std::vector<VkImage> mSwapChainImages;
        std::vector<VkImageView> mSwapChainImageViews;
        std::vector<VkSemaphore> mRenderCompleteSemaphores;
    };

}
