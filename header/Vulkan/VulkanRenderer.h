#pragma once

#include "VulkanContext.h"
#include "VulkanFrameManager.h"
#include "VulkanPresenter.h"

#include <memory>

namespace VKRE {

    class VulkanRenderer {
    public:
        VulkanRenderer(std::shared_ptr<VulkanContext> context, const Window* window);

        void Render();

    private:
        // NOTE: This is temporary!
        void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
        VkImageSubresourceRange ImageSubSourceRange(VkImageAspectFlags aspectMask);

    private:
        std::shared_ptr<VulkanContext> mContext;
        std::unique_ptr<VulkanFrameManager> mFrameManager;
        std::unique_ptr<VulkanPresenter> mPresenter;
    };

}
