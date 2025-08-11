#include <Vulkan/VulkanFrameManager.h>

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace VKRE {

    VulkanFrameManager::VulkanFrameManager(std::shared_ptr<VulkanContext> context, uint32_t framesInFlight) 
        : mContext(context), mFrames(framesInFlight) {
            CreateCommandPools();
            CreateSyncObjects();
        }

    VulkanFrameManager::~VulkanFrameManager() {
        auto device = mContext->GetLogicalDevice().handle;

        for (auto& frame : mFrames) {
            VK_CHECK(vkWaitForFences(mContext->GetLogicalDevice().handle, 1, &frame.waitFence, true, 1000000000));
            VK_CHECK(vkResetFences(mContext->GetLogicalDevice().handle, 1, &frame.waitFence));

            vkDestroyCommandPool(device, frame.commandPool, nullptr);
            vkDestroySemaphore(device, frame.presentCompleteSemaphore, nullptr);
            vkDestroyFence(device, frame.waitFence, nullptr);
            frame.deletionQueue.Flush();
        }
    }

    void VulkanFrameManager::CreateCommandPools() {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = mContext->GetQueueFamilies().graphicsFamily.value();

        for (auto& frame : mFrames) {
            VK_CHECK(vkCreateCommandPool(mContext->GetLogicalDevice().handle, &poolInfo, nullptr, &frame.commandPool));

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = frame.commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            VK_CHECK(vkAllocateCommandBuffers(mContext->GetLogicalDevice().handle, &allocInfo, &frame.commandBuffer));
        }
    }

    void VulkanFrameManager::CreateSyncObjects() {
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (auto& frame : mFrames) {
            VK_CHECK(vkCreateFence(mContext->GetLogicalDevice().handle, &fenceCreateInfo, nullptr, &frame.waitFence));
            VK_CHECK(vkCreateSemaphore(mContext->GetLogicalDevice().handle, &semaphoreCreateInfo, nullptr, &frame.presentCompleteSemaphore));
        }
    }

}

