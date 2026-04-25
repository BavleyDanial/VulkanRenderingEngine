#include <Vulkan/VulkanPresenter.h>

#include <cassert>

namespace  VKRE {

    VulkanPresenter::VulkanPresenter(VulkanContext& context)
        :mContext(context) {
        CreateSwapChain();
    }

    VulkanPresenter::~VulkanPresenter() {
        DestroySwapChain();
    }

    void VulkanPresenter::CreateSwapChain() {
        auto [width, height] = mContext.GetWindowContext()->GetFrameBufferExtents();

        VulkanSwapChainBuilder swapChainBuilder(mContext.GetInstance(), mContext.GetSurface(), mContext.GetPhysicalDevice(), mContext.GetLogicalDevice());
        std::optional<VulkanSwapChain> swapChain = swapChainBuilder.SetDesiredExtent(width, height)
                                                    .SetDesiredImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                                    .SetDesiredFormat(VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
                                                    .SetDesiredPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
                                                    .Build();
        if (swapChain.has_value()) {
            mSwapChain = swapChain.value();
        } else {
            std::println("Failed to Create Vulkan Swapchain!");
            abort();
        }

        GetSwapChainImages();
        GetSwapChainImageViews();

        mRenderCompleteSemaphores.resize(mSwapChainImages.size());
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (auto& semaphore : mRenderCompleteSemaphores) {
            VK_CHECK(vkCreateSemaphore(mSwapChain.deviceHandle, &semaphoreCreateInfo, nullptr, &semaphore));
        }

    }

    void VulkanPresenter::GetSwapChainImages() {
        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(mSwapChain.deviceHandle, mSwapChain.handle, &imageCount, nullptr);

        mSwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(mSwapChain.deviceHandle, mSwapChain.handle, &imageCount, mSwapChainImages.data());
    }

    void VulkanPresenter::GetSwapChainImageViews() {
        mSwapChainImageViews.resize(mSwapChainImages.size());

        for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = mSwapChainImages[i];

            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = mSwapChain.imageFormat.format;

            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(mSwapChain.deviceHandle, &imageViewCreateInfo, nullptr, &mSwapChainImageViews[i]));
        }
    }

    void VulkanPresenter::DestroySwapChain() {
        vkDeviceWaitIdle(mSwapChain.deviceHandle);
        for (auto& semaphore : mRenderCompleteSemaphores) {
            vkDestroySemaphore(mSwapChain.deviceHandle, semaphore, nullptr);
        }
        mSwapChain.DestroyImageViews(mSwapChainImageViews);
        mSwapChain.Destroy();
    }

    void VulkanPresenter::ResizeSwapChain() {
        DestroySwapChain();
        CreateSwapChain();
    }
}

