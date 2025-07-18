#include <Vulkan/VulkanSwapChain.h>

#include <Vulkan/VulkanContext.h>
#include <Vulkan/VulkanPhysicalDevice.h>
#include <Vulkan/VulkanLogicalDevice.h>

#include <Engine.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <cstdint>
#include <cassert>
#include <print>
#include <vulkan/vulkan_core.h>

namespace VKRE {

    std::vector<VkImage> VulkanSwapChain::GetImages() const {
        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(deviceHandle, handle, &imageCount, nullptr);

        std::vector<VkImage> images(imageCount);
        vkGetSwapchainImagesKHR(deviceHandle, handle, &imageCount, images.data());
        return images;
    }

    std::vector<VkImageView> VulkanSwapChain::GetImageViews(const std::vector<VkImage>& images) const {
        std::vector<VkImageView> imageViews(images.size());

        for (size_t i = 0; i < imageViews.size(); i++) {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = images[i];

            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = imageFormat.format;

            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(deviceHandle, &imageViewCreateInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
                assert("ERROR: coudln't create Image View!");
            }
        }

        return imageViews;
    }

    void VulkanSwapChain::DestroyImageViews(std::vector<VkImageView>& imageViews) {
        for (auto& imageView : imageViews) {
            vkDestroyImageView(deviceHandle, imageView, nullptr);
        }
    }

    VulkanSwapChainBuilder::VulkanSwapChainBuilder(VkInstance instance, VkSurfaceKHR surface, const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& logicalDevice)
        :mInstance(instance), mSurface(surface), mPhysicalDevice(physicalDevice), mLogicalDevice(logicalDevice) {

        if (std::find(mPhysicalDevice.extensions.begin(), mPhysicalDevice.extensions.end(), VK_KHR_SWAPCHAIN_EXTENSION_NAME) != mPhysicalDevice.extensions.end()) {
            assert("Physical device doesn't support swapchains");
        }
    }

    std::optional<VulkanSwapChain> VulkanSwapChainBuilder::Build() {
        VulkanSwapChain swapChain{};
        SwapChainSupportDetails swapChainSupportDetails = QuerySupport();

        if (swapChainSupportDetails.formats.empty() || swapChainSupportDetails.presentModes.empty()) {
            return std::nullopt;
        }

        swapChain.imageFormat = ChooseSurfaceFormat(swapChainSupportDetails.formats);
        swapChain.presentMode = ChooseSwapPresentMode(swapChainSupportDetails.presentModes);
        swapChain.extent = ChooseSwapExtent(swapChainSupportDetails.capabilities);
        swapChain.minImageCount = ChooseSwapImageCount(swapChainSupportDetails.capabilities);
        swapChain.imageUsage = mSwapChainConfig.imageUsage; // TODO: Change this to a function to check if usage is supported

        QueueFamilyIndinces indices = mPhysicalDevice.queueFamilyIndicies;
        if (!indices.IsComplete()) {
            assert("Error: Graphics and Present indices are not available on this physical device!");
            return std::nullopt;
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo{};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = mSurface;
        swapChainCreateInfo.minImageCount = swapChain.minImageCount;
        swapChainCreateInfo.imageColorSpace = swapChain.imageFormat.colorSpace;
        swapChainCreateInfo.imageFormat = swapChain.imageFormat.format;
        swapChainCreateInfo.imageExtent = swapChain.extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = mSwapChainConfig.imageUsage;

        if (indices.graphicsFamily.value() != indices.presentFamily.value()) {
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;
            swapChainCreateInfo.pQueueFamilyIndices = nullptr;
        }

        swapChainCreateInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = swapChain.presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(mLogicalDevice.handle, &swapChainCreateInfo, nullptr, &swapChain.handle) != VK_SUCCESS) {
            assert("Error: Couldn't create swapchain!");
            return std::nullopt;
        }

        swapChain.deviceHandle = mLogicalDevice.handle;
        return swapChain;
    }

    VulkanSwapChainBuilder::SwapChainSupportDetails VulkanSwapChainBuilder::QuerySupport() {
        SwapChainSupportDetails details;

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice.handle, mSurface, &formatCount, nullptr);
        if (formatCount > 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice.handle, mSurface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice.handle, mSurface, &presentModeCount, nullptr);
        if (presentModeCount > 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice.handle, mSurface, &presentModeCount, details.presentModes.data());
        }

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice.handle, mSurface, &details.capabilities);
        return details;
    }

    VkSurfaceFormatKHR VulkanSwapChainBuilder::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaialbleFormats) const {
        for (const auto& availableFormat : avaialbleFormats) {
            if (availableFormat.format == mSwapChainConfig.surfaceFormat.format && availableFormat.colorSpace == mSwapChainConfig.surfaceFormat.colorSpace) {
                return availableFormat;
            }
        }

        std::println("Warning: Couldn't find desired surface format");
        return avaialbleFormats[0];
    }

    VkPresentModeKHR VulkanSwapChainBuilder::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes) const {
        for (const auto& availableMode : availableModes) {
            if (availableMode == mSwapChainConfig.presentMode) {
                return availableMode;
            }
        }

        std::println("Warning: Couldn't find desired swap present mode");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapChainBuilder::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent = mSwapChainConfig.extent;

        actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    uint32_t VulkanSwapChainBuilder::ChooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities) const {
        uint32_t imageCount = mSwapChainConfig.imageCount;
        if (imageCount == 0) {
            imageCount = capabilities.minImageCount + 1;
        }

        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            std::println("Warning: swapchain is only capable of {0} max image count", capabilities.maxImageCount);
            imageCount = capabilities.maxImageCount;
        }

        if (capabilities.minImageCount > 0 && imageCount < capabilities.minImageCount) {
            std::println("Warning: swapchain is only capable of {0} min image count", capabilities.minImageCount);
            imageCount = capabilities.minImageCount;
        }

        return imageCount;
    }

    VulkanSwapChainBuilder& VulkanSwapChainBuilder::SetDesiredFormat(const VkSurfaceFormatKHR& format) {
        mSwapChainConfig.surfaceFormat = format;
        return *this;
    }

    VulkanSwapChainBuilder& VulkanSwapChainBuilder::SetDesiredPresentMode(const VkPresentModeKHR& mode) {
        mSwapChainConfig.presentMode = mode;
        return *this;
    }

    VulkanSwapChainBuilder& VulkanSwapChainBuilder::SetDesiredExtent(uint32_t width, uint32_t height) {
        mSwapChainConfig.extent = { width, height };
        return *this;
    }

    VulkanSwapChainBuilder& VulkanSwapChainBuilder::SetDesiredImageCount(uint32_t count) {
        mSwapChainConfig.imageCount = count;
        return *this;
    }

    VulkanSwapChainBuilder& VulkanSwapChainBuilder::SetDesiredImageUsage(VkImageUsageFlags flag) {
        mSwapChainConfig.imageUsage = flag;
       return *this;
    }

}
