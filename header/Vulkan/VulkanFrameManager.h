#pragma once

#include "VulkanContext.h"

#include <memory>

namespace VKRE {

    struct VulkanFrameData {
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore presentCompleteSemaphore;
        VkFence waitFence;
    };

    class VulkanFrameManager {
    public:
        VulkanFrameManager(std::shared_ptr<VulkanContext> context, uint32_t framesInFlight = 2);
        ~VulkanFrameManager();

        VulkanFrameData& GetCurrentFrame() { return mFrames[mCurrentFrame % mFrames.size()]; }
        uint64_t GetTotalFramesCount() const { return mCurrentFrame; }
        void AdvanceFrame() { mCurrentFrame++; }

    private:
        void CreateCommandPools();
        void CreateSyncObjects();

    private:
        std::shared_ptr<VulkanContext> mContext;
        std::vector<VulkanFrameData> mFrames;
        uint64_t mCurrentFrame = 0;
    };

}
