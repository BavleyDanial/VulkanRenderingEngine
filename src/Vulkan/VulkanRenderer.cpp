#include <Vulkan/VulkanRenderer.h>

#include <Engine.h>
#include <glm/glm.hpp>

#include <cassert>
#include <memory>
#include <vulkan/vulkan_core.h>

#include <Vulkan/VulkanImGuiPass.h>

namespace VKRE {

    VulkanRenderer::VulkanRenderer(VulkanContext& context)
    :mContext(context) {
        mFrameManager = std::make_unique<VulkanFrameManager>(mContext);
        mPresenter = std::make_unique<VulkanPresenter>(mContext);

        CreateDrawImage();
        InitPasses();
        InitDescriptors();
        InitPipelines();

        mDeletionQueue.PushDeleteFunc([this]() { mDrawImage->Release(); });
    }

    VulkanRenderer::~VulkanRenderer() {
        mPresenter.reset();
        mFrameManager.reset();
        mDeletionQueue.Flush();
    }

    void VulkanRenderer::CreateDrawImage() {
        auto [width, height] = mPresenter->GetSwapChain().extent;
        VkExtent3D drawImageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
        VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
        VkImageUsageFlags drawImageUsages{};
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VmaAllocationCreateInfo drawImageAllocInfo = {};
        drawImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        drawImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        mDrawImage = std::make_unique<VulkanImage2D>(mContext);
        mDrawImage->CreateImage(format, drawImageUsages, drawImageExtent, VK_IMAGE_ASPECT_COLOR_BIT, drawImageAllocInfo);
    }

    void VulkanRenderer::ReCreateDrawImage() {
        mDrawImage->Release();
        mGlobalDescriptorAllocator.ClearDescriptors(mContext.GetLogicalDevice().handle);
        CreateDrawImage();
        InitDrawImageDescriptor();
    }

    void VulkanRenderer::InitPasses() {
        mPasses.push_back(std::make_unique<VulkanImGuiPass>(mContext, *mPresenter));
    }

    void VulkanRenderer::InitDescriptors() {
        std::vector<DescriptorAllocator::PoolSizeRatio> poolSizes = {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
        };

        mGlobalDescriptorAllocator.InitPool(mContext.GetLogicalDevice().handle, 10, poolSizes);

        {   // Compute Descriptor Set
            DescriptorLayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0);
            mDrawImageDescriptorLayout = layoutBuilder.Build(mContext.GetLogicalDevice().handle, VK_SHADER_STAGE_COMPUTE_BIT);
        }

        InitDrawImageDescriptor();

        mDeletionQueue.PushDeleteFunc([this]() {
            mGlobalDescriptorAllocator.DestroyPool(mContext.GetLogicalDevice().handle);
            vkDestroyDescriptorSetLayout(mContext.GetLogicalDevice().handle, mDrawImageDescriptorLayout, nullptr);
        });
    }

    void VulkanRenderer::InitDrawImageDescriptor() {
        mDrawImageDescriptors = mGlobalDescriptorAllocator.Allocate(mContext.GetLogicalDevice().handle, mDrawImageDescriptorLayout);

        VkDescriptorImageInfo imgInfo{};
        imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imgInfo.imageView = mDrawImage->GetImageInfo().imageView;

        VkWriteDescriptorSet drawImageWrite = {};
        drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        drawImageWrite.pNext = nullptr;

        drawImageWrite.dstBinding = 0;
        drawImageWrite.dstSet = mDrawImageDescriptors;
        drawImageWrite.descriptorCount = 1;
        drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        drawImageWrite.pImageInfo = &imgInfo;

        vkUpdateDescriptorSets(mContext.GetLogicalDevice().handle, 1, &drawImageWrite, 0, nullptr);
    }

    void VulkanRenderer::InitPipelines() {
        InitBackgroundPipelines();
    }

    void VulkanRenderer::InitBackgroundPipelines() {
        VkPipelineLayoutCreateInfo computeLayout{};
        computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        computeLayout.pSetLayouts = &mDrawImageDescriptorLayout;
        computeLayout.setLayoutCount = 1;

        VkPushConstantRange pushConstant{};
        pushConstant.offset = 0;
        pushConstant.size = sizeof(ComputePushConstants) ;
        pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        computeLayout.pPushConstantRanges = &pushConstant;
        computeLayout.pushConstantRangeCount = 1;

        VK_CHECK(vkCreatePipelineLayout(mContext.GetLogicalDevice().handle, &computeLayout, nullptr, &mGradientPipelineLayout));

        // TODO: Make shaders system that automatically loads shaders based on a config file or something instead of hard-coding the relative path
        VkShaderModule gradientShader = VulkanUtils::LoadShader(mContext.GetLogicalDevice().handle, "res/shaders/gradient.spv");
        if (!gradientShader) {
            std::println("Error when building the compute shader");
        }

        VkPipelineShaderStageCreateInfo stageinfo{};
        stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageinfo.pNext = nullptr;
        stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stageinfo.module = gradientShader;
        stageinfo.pName = "main";

        VkComputePipelineCreateInfo computePipelineCreateInfo{};
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.pNext = nullptr;
        computePipelineCreateInfo.layout = mGradientPipelineLayout;
        computePipelineCreateInfo.stage = stageinfo;

        ComputeEffect gradient;
        gradient.layout = mGradientPipelineLayout;
        gradient.name = "gradient";
        gradient.data = {};

        gradient.data.data1 = glm::vec4(1, 0, 0, 1);
        gradient.data.data2 = glm::vec4(0, 0, 1, 1);

        VK_CHECK(vkCreateComputePipelines(mContext.GetLogicalDevice().handle, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline));

        ComputeEffect sky;
        sky.layout = mGradientPipelineLayout;
        sky.name = "gradient";
        sky.data = {};

        sky.data.data1 = glm::vec4(0.1, 0.2, 0.4 ,0.97);

        VK_CHECK(vkCreateComputePipelines(mContext.GetLogicalDevice().handle, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &sky.pipeline));

        backgroundEffects.push_back(gradient);
        backgroundEffects.push_back(sky);

        vkDestroyShaderModule(mContext.GetLogicalDevice().handle, gradientShader, nullptr);
        mDeletionQueue.PushDeleteFunc([this]() {
            vkDestroyPipelineLayout(mContext.GetLogicalDevice().handle, mGradientPipelineLayout, nullptr);
            vkDestroyPipeline(mContext.GetLogicalDevice().handle, backgroundEffects[0].pipeline, nullptr);
            vkDestroyPipeline(mContext.GetLogicalDevice().handle, backgroundEffects[1].pipeline, nullptr);
        });
    }

    void VulkanRenderer::Render() {
        if (Engine::GetInstance().hasResized) {
            ReSize();
            Engine::GetInstance().hasResized = false;
            return;
        }

        VulkanFrameData& frame = mFrameManager->GetCurrentFrame();
        VK_CHECK(vkWaitForFences(mContext.GetLogicalDevice().handle, 1, &frame.waitFence, true, UINT64_MAX));
        frame.deletionQueue.Flush();

        uint32_t swapchainImageIndex = 0;
        VkResult acquireResult = vkAcquireNextImageKHR(mContext.GetLogicalDevice().handle, mPresenter->GetSwapChain().handle, UINT64_MAX, frame.presentCompleteSemaphore, nullptr, &swapchainImageIndex);
        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
            ReSize();
            VK_CHECK(vkResetFences(mContext.GetLogicalDevice().handle, 1, &frame.waitFence));
            return;
        }
        VK_CHECK(acquireResult);
        VK_CHECK(vkResetFences(mContext.GetLogicalDevice().handle, 1, &frame.waitFence));


        // NOTE: The following is temporary!
        VkCommandBuffer cmd = frame.commandBuffer;
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfo.pNext = nullptr;
        cmdBufferBeginInfo.pInheritanceInfo = nullptr;
        cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBufferBeginInfo));

        ImageUtils::TransitionImage(cmd, mDrawImage->GetImageInfo().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        // TODO: Replace this with passes
        ClearImage(cmd);
        DrawGradientBackground(cmd);
        ImageUtils::TransitionImage(cmd, mDrawImage->GetImageInfo().image,VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        VkImage swapChainImage = mPresenter->GetImages()[swapchainImageIndex];
        VkExtent2D drawImageExtent = { mDrawImage->GetImageInfo().extent.width, mDrawImage->GetImageInfo().extent.height };
        ImageUtils::TransitionImage(cmd, swapChainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        ImageUtils::CopyImage(cmd, mDrawImage->GetImageInfo().image, swapChainImage, drawImageExtent, mPresenter->GetSwapChain().extent);
        ImageUtils::TransitionImage(cmd, swapChainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        mPasses.back()->Execute(cmd, { swapchainImageIndex, nullptr} ); // Execute ImGuiPass

        ImageUtils::TransitionImage(cmd, swapChainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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

        VK_CHECK(vkQueueSubmit2(mContext.GetGraphicsQueue(), 1, &info, frame.waitFence));
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.pSwapchains = &mPresenter->GetSwapChain().handle;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &mPresenter->GetRenderCompleteSemaphore(swapchainImageIndex);
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &swapchainImageIndex;
        VK_CHECK(vkQueuePresentKHR(mContext.GetGraphicsQueue(), &presentInfo));

        mFrameManager->AdvanceFrame();
    }

    void VulkanRenderer::ReSize() {
        vkDeviceWaitIdle(mContext.GetLogicalDevice().handle);
        mPresenter->ResizeSwapChain();
        ReCreateDrawImage();
    }

    void VulkanRenderer::DrawGradientBackground(VkCommandBuffer cmd) {
        ComputeEffect& effect = backgroundEffects[currentBackgroundEffect];
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mGradientPipelineLayout, 0, 1, &mDrawImageDescriptors, 0, nullptr);
        vkCmdPushConstants(cmd, mGradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
        vkCmdDispatch(cmd, std::ceil(mDrawImage->GetImageInfo().extent.width / 16.0), std::ceil(mDrawImage->GetImageInfo().extent.height / 16.0), 1);
    }

    void VulkanRenderer::ClearImage(VkCommandBuffer cmd) {
        VkClearColorValue clearValue;
        clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        VkImageSubresourceRange clearRange = ImageUtils::ImageSubSourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
        vkCmdClearColorImage(cmd, mDrawImage->GetImageInfo().image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
    }

}
