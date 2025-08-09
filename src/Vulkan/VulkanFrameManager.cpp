#include <Vulkan/VulkanFrameManager.h>

#include <cassert>
#include <vulkan/vulkan_core.h>

#include "Vulkan/VulkanPresenter.h"

namespace VKRE {

    VulkanFrameManager::VulkanFrameManager(std::shared_ptr<VulkanContext> context, uint32_t framesInFlight) 
        : mContext(context), mFrames(framesInFlight) {
            CreateCommandPools();
            CreateSyncObjects();
        }

    VulkanFrameManager::~VulkanFrameManager() {
        auto device = mContext->GetLogicalDevice().handle;
        
        for (auto& frame : mFrames) {
            if (vkWaitForFences(mContext->GetLogicalDevice().handle, 1, &frame.waitFence, true, 1000000000) != VK_SUCCESS) {
                assert("Error: Render Fence Timeout!");
            }

            if (vkResetFences(mContext->GetLogicalDevice().handle, 1, &frame.waitFence) != VK_SUCCESS) {
                assert("Error: Couldn't reset render fence!");
            }
            
            vkDestroyCommandPool(device, frame.commandPool, nullptr);
            vkDestroySemaphore(device, frame.presentCompleteSemaphore, nullptr);
            vkDestroyFence(device, frame.waitFence, nullptr);
        }
    }

    void VulkanFrameManager::CreateCommandPools() {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = mContext->GetQueueFamilies().graphicsFamily.value();

        for (auto& frame : mFrames) {
            if (vkCreateCommandPool(mContext->GetLogicalDevice().handle, &poolInfo, nullptr, &frame.commandPool) != VK_SUCCESS) {
                assert("Error: Couldn't create command pool!");
            }

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = frame.commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            if (vkAllocateCommandBuffers(mContext->GetLogicalDevice().handle, &allocInfo, &frame.commandBuffer) != VK_SUCCESS) {
                assert("Error: Couldn't create command buffer!");
            }
        }
    }

    void VulkanFrameManager::CreateSyncObjects() {
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (int i = 0; i < mFrames.size(); i++) {
            if (vkCreateFence(mContext->GetLogicalDevice().handle, &fenceCreateInfo, nullptr, &mFrames[i].waitFence) != VK_SUCCESS) {
                assert("Couldn't create render fence!");
            }

            if (vkCreateSemaphore(mContext->GetLogicalDevice().handle, &semaphoreCreateInfo, nullptr, &mFrames[i].presentCompleteSemaphore) != VK_SUCCESS) {
                assert("Couldn't create render semaphore!");
            }
        }
    }

}

