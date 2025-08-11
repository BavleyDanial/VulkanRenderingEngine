#include <Vulkan/VulkanRenderer.h>

#include <Engine.h>
#include <glm/glm.hpp>

#include <cassert>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace VKRE {

    VulkanRenderer::VulkanRenderer(std::shared_ptr<VulkanContext> context)
    :mContext(context) {
        mFrameManager = std::make_unique<VulkanFrameManager>(context);
        mPresenter = std::make_unique<VulkanPresenter>(context);
    }

    VulkanRenderer::~VulkanRenderer() {
        mPresenter.reset();
        mFrameManager.reset();
    }

    void VulkanRenderer::Render() {
        VulkanFrameData& frame = mFrameManager->GetCurrentFrame();

        VK_CHECK(vkWaitForFences(mContext->GetLogicalDevice().handle, 1, &frame.waitFence, true, UINT64_MAX));
        frame.deletionQueue.Flush();

        if (Engine::GetInstance().hasResized) {
            mPresenter->ResizeSwapChain();
            Engine::GetInstance().hasResized = false;
            return;
        }

        uint32_t swapchainImageIndex = 0;
        VK_CHECK(vkAcquireNextImageKHR(mContext->GetLogicalDevice().handle, mPresenter->GetSwapChain().handle, UINT64_MAX, frame.presentCompleteSemaphore, nullptr, &swapchainImageIndex));

        VK_CHECK(vkResetFences(mContext->GetLogicalDevice().handle, 1, &frame.waitFence));

        // NOTE: The following is temporary!
        VkCommandBuffer cmd = frame.commandBuffer;
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfo.pNext = nullptr;
        cmdBufferBeginInfo.pInheritanceInfo = nullptr;
        cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBufferBeginInfo));
        TransitionImage(cmd, mPresenter->GetImages()[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        VkClearColorValue clearValue;
        clearValue = { { 0.0f, 0.0f, 1.0f, 1.0f } };

        VkImageSubresourceRange clearRange = ImageSubSourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
        vkCmdClearColorImage(cmd, mPresenter->GetImages()[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
        TransitionImage(cmd, mPresenter->GetImages()[swapchainImageIndex],VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        VK_CHECK(vkEndCommandBuffer(cmd));

        VkSemaphoreSubmitInfo presentCompleteSemaphoreSubmitInfo{};
        presentCompleteSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        presentCompleteSemaphoreSubmitInfo.semaphore = frame.presentCompleteSemaphore;
        presentCompleteSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;

        VkSemaphoreSubmitInfo renderCompleteSemaphoreSubmitInfo{};
        renderCompleteSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        renderCompleteSemaphoreSubmitInfo.semaphore = mPresenter->GetRenderCompleteSemaphore(swapchainImageIndex);
        renderCompleteSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

        VkCommandBufferSubmitInfo cmdSubmitInfo;
        cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdSubmitInfo.pNext = nullptr;
        cmdSubmitInfo.commandBuffer = cmd;
        cmdSubmitInfo.deviceMask = 0;

        VkSubmitInfo2 info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        info.pNext = nullptr;
        info.waitSemaphoreInfoCount = 1;
        info.pWaitSemaphoreInfos = &presentCompleteSemaphoreSubmitInfo;
        info.signalSemaphoreInfoCount = 1;
        info.pSignalSemaphoreInfos = &renderCompleteSemaphoreSubmitInfo;
        info.commandBufferInfoCount = 1;
        info.pCommandBufferInfos = &cmdSubmitInfo;

        VK_CHECK(vkQueueSubmit2(mContext->GetGraphicsQueue(), 1, &info, frame.waitFence));
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.pSwapchains = &mPresenter->GetSwapChain().handle;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &mPresenter->GetRenderCompleteSemaphore(swapchainImageIndex);
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &swapchainImageIndex;
        VK_CHECK(vkQueuePresentKHR(mContext->GetGraphicsQueue(), &presentInfo));

        mFrameManager->AdvanceFrame();
    }

    void VulkanRenderer::TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) {
        VkImageMemoryBarrier2 imageBarrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        imageBarrier.pNext = nullptr;

        imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

        imageBarrier.oldLayout = currentLayout;
        imageBarrier.newLayout = newLayout;

        VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrier.subresourceRange = ImageSubSourceRange(aspectMask);
        imageBarrier.image = image;

        VkDependencyInfo depInfo {};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.pNext = nullptr;

        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &imageBarrier;

        vkCmdPipelineBarrier2(cmd, &depInfo);
    }

    VkImageSubresourceRange VulkanRenderer::ImageSubSourceRange(VkImageAspectFlags aspectMask) {
        VkImageSubresourceRange subImage {};
        subImage.aspectMask = aspectMask;
        subImage.baseMipLevel = 0;
        subImage.levelCount = VK_REMAINING_MIP_LEVELS;
        subImage.baseArrayLayer = 0;
        subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

        return subImage;
    }

}
