#include <Vulkan/VulkanFrameManager.h>

#include <cassert>

namespace VKRE {

    VulkanFrameManager::VulkanFrameManager(VulkanContext& context, uint32_t framesInFlight) 
        : mContext(context), mFrames(framesInFlight) {
            CreateCommandPools();
            CreateSyncObjects();

            for (auto& frame : mFrames) {
                std::vector<VulkanPoolSizeRatio> frameSizes = {
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
                };

                frame.FrameDescriptors = VulkanGrowableDescriptorAllocator{};
                frame.FrameDescriptors.InitPool(mContext.GetLogicalDevice().handle, 1000, frameSizes);
            }
        }

    VulkanFrameManager::~VulkanFrameManager() {
        auto device = mContext.GetLogicalDevice().handle;

        for (auto& frame : mFrames) {
            VK_CHECK(vkWaitForFences(mContext.GetLogicalDevice().handle, 1, &frame.WaitFence, true, 1000000000));
            VK_CHECK(vkResetFences(mContext.GetLogicalDevice().handle, 1, &frame.WaitFence));

            vkDestroyCommandPool(device, frame.CommandPool, nullptr);
            vkDestroySemaphore(device, frame.PresentCompleteSemaphore, nullptr);
            vkDestroyFence(device, frame.WaitFence, nullptr);

            frame.FrameDescriptors.DestroyPools(device);
        }
    }

    void VulkanFrameManager::ClearFramePools() {
        auto device = mContext.GetLogicalDevice().handle;
        GetCurrentFrame().FrameDescriptors.ClearPools(device);
    }

    void VulkanFrameManager::CreateCommandPools() {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = mContext.GetQueueFamilies().Get(QueueCapability::Graphics);

        for (auto& frame : mFrames) {
            VK_CHECK(vkCreateCommandPool(mContext.GetLogicalDevice().handle, &poolInfo, nullptr, &frame.CommandPool));

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = frame.CommandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            VK_CHECK(vkAllocateCommandBuffers(mContext.GetLogicalDevice().handle, &allocInfo, &frame.CommandBuffer));
        }
    }

    void VulkanFrameManager::CreateSyncObjects() {
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (auto& frame : mFrames) {
            VK_CHECK(vkCreateFence(mContext.GetLogicalDevice().handle, &fenceCreateInfo, nullptr, &frame.WaitFence));
            VK_CHECK(vkCreateSemaphore(mContext.GetLogicalDevice().handle, &semaphoreCreateInfo, nullptr, &frame.PresentCompleteSemaphore));
        }
    }

}

