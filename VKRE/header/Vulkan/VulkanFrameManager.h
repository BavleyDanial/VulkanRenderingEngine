#pragma once

#include <Vulkan/VulkanContext.h>
#include <Vulkan/VulkanDescriptors.h>

namespace VKRE {

    struct VulkanFrameData {
        VkCommandPool CommandPool;
        VkCommandBuffer CommandBuffer;
        VkSemaphore PresentCompleteSemaphore;
        VkFence WaitFence;

        VulkanGrowableDescriptorAllocator FrameDescriptors;
    };

    class VulkanFrameManager {
    public:
        VulkanFrameManager(VulkanContext& context, uint32_t framesInFlight = 2);
        ~VulkanFrameManager();

        VulkanFrameData& GetCurrentFrame() { return mFrames[mCurrentFrame % mFrames.size()]; }
        uint64_t GetTotalFramesCount() const { return mCurrentFrame; }
        uint32_t GetFramesInFlight() const { return static_cast<uint32_t>(mFrames.size()); }
        void AdvanceFrame() { mCurrentFrame++; }

        void ClearFramePools();

    private:
        void CreateCommandPools();
        void CreateSyncObjects();

    private:
        VulkanContext& mContext;
        std::vector<VulkanFrameData> mFrames;
        uint64_t mCurrentFrame = 0;
    };

}
