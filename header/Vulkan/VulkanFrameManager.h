#pragma once

#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace VKRE {

    struct VulkanFrameData {
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore presentCompleteSemaphore;
        VkFence waitFence;
        VulkanUtils::DeletionQueue deletionQueue;
    };

    class VulkanFrameManager {
    public:
        VulkanFrameManager(VulkanContext& context, uint32_t framesInFlight = 2);
        ~VulkanFrameManager();

        VulkanFrameData& GetCurrentFrame() { return mFrames[mCurrentFrame % mFrames.size()]; }
        uint64_t GetTotalFramesCount() const { return mCurrentFrame; }
        void AdvanceFrame() { mCurrentFrame++; }

    private:
        void CreateCommandPools();
        void CreateSyncObjects();

    private:
        VulkanContext& mContext;
        std::vector<VulkanFrameData> mFrames;
        uint64_t mCurrentFrame = 0;
    };

}
