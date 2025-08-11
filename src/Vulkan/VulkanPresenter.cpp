#include <Vulkan/VulkanPresenter.h>

#include <cassert>

namespace  VKRE {

    VulkanPresenter::VulkanPresenter(std::shared_ptr<VulkanContext> context)
        :mContext(context) {
        CreateSwapChain();
    }

    VulkanPresenter::~VulkanPresenter() {
        DestroySwapChain();
    }

    void VulkanPresenter::CreateSwapChain() {
        auto [width, height] = mContext->GetWindowContext()->GetFrameBufferExtents();

        VulkanSwapChainBuilder swapChainBuilder(mContext->GetInstance(), mContext->GetSurface(), mContext->GetPhysicalDevice(), mContext->GetLogicalDevice());
        std::optional<VulkanSwapChain> swapChain = swapChainBuilder.SetDesiredExtent(width, height)
                                                    .SetDesiredImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                                    .SetDesiredFormat(VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
                                                    .SetDesiredPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
                                                    .Build();
        if (swapChain.has_value()) {
            mSwapChain = swapChain.value();
        } else {
            std::println("Failed to Create Vulkan Swapchain!");
            abort();
        }

        mSwapChainImages = mSwapChain.GetImages();
        mSwapChainImageViews = mSwapChain.GetImageViews(mSwapChain.GetImages());

        mRenderCompleteSemaphores.resize(mSwapChainImages.size());
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (auto& semaphore : mRenderCompleteSemaphores) {
            VK_CHECK(vkCreateSemaphore(mSwapChain.deviceHandle, &semaphoreCreateInfo, nullptr, &semaphore));
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

