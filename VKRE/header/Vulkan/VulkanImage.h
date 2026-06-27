#pragma once

#include <Vulkan/VulkanUtils.h>
#include <Vulkan/VulkanContext.h>

namespace VKRE {

    struct VulkanImageData {
        VkImage image{};
        VkImageView imageView{};
        VmaAllocation allocation{};
        VkFormat format{};
        VkExtent3D extent{};
    };

    namespace ImageUtils {
        VulkanImageData ReCreateImage(VulkanContext& context, VulkanImageData* oldImageData, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, VmaAllocationCreateInfo& info);
        VulkanImageData CreateImage(VulkanContext& context, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, VmaAllocationCreateInfo& info);
        void ReleaseImage(VulkanContext& context, VulkanImageData* image);

        void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
        void CopyImage(VkCommandBuffer cmd, VkImage src, VkImage dest, VkExtent2D srcSize, VkExtent2D dstSize);
        VkImageSubresourceRange ImageSubSourceRange(VkImageAspectFlags aspectMask);
    };

};
