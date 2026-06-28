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
        uint32_t mipLevels = 1;
        uint32_t arrayLevels = 1;
    };

    namespace ImageUtils {

        VulkanImageData ReCreateImage(VulkanContext& context, VulkanImageData* oldImageData, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VmaAllocationCreateInfo& info);
        VulkanImageData CreateImage(VulkanContext& context, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VmaAllocationCreateInfo& info);

        VulkanImageData ReCreateImageCube(VulkanContext& context, VulkanImageData* oldImageData, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VmaAllocationCreateInfo& info);
        VulkanImageData CreateImageCube(VulkanContext& context, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VmaAllocationCreateInfo& info);

        void ReleaseImage(VulkanContext& context, VulkanImageData* image);

        void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
        void TransitionImageMip(VkCommandBuffer cmd, VkImage image, uint32_t mipLevel, VkImageLayout currentLayout, VkImageLayout newLayout);
        void CopyImage(VkCommandBuffer cmd, VkImage src, VkImage dest, VkExtent2D srcSize, VkExtent2D dstSize, uint32_t srcMip = 0, uint32_t dstMip = 0);
        VkImageSubresourceRange ImageSubSourceRange(VkImageAspectFlags aspectMask, uint32_t mipLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t arrayLayer = 0, uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS);

    }

};
