#pragma once

#include "VulkanImage.h"

namespace VKRE {

    struct FrameInfo {
        uint32_t swapchainImageIdx = 0;
        VulkanImage2D* drawImage = nullptr;
    };

    class IVulkanRenderPass {
    public:
        virtual ~IVulkanRenderPass() = default;
        virtual void Execute(VkCommandBuffer cmd, const FrameInfo& frameInfo) = 0;
        virtual void OnImGui() {}
    };

}
