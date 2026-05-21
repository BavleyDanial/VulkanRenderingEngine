#pragma once

#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace VKRE {

    struct VulkanImageData {
        VkImage image{};
        VkImageView imageView{};
        VmaAllocation allocation{};
        VkFormat format{};
        VkExtent3D extent{};
    };

    class VulkanImage2D {
    public:
        VulkanImage2D(VulkanContext& context);
        ~VulkanImage2D();

        VulkanImageData& GetImageInfo() { return mImageInfo; }

        void ReCreateImage(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, VmaAllocationCreateInfo& info);
        void CreateImage(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, VmaAllocationCreateInfo& info);
        void Release();

    private:
        VulkanContext& mContext;
        VulkanImageData mImageInfo;
    };


    namespace ImageUtils {
        void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
        void CopyImage(VkCommandBuffer cmd, VkImage src, VkImage dest, VkExtent2D srcSize, VkExtent2D dstSize);
        VkImageSubresourceRange ImageSubSourceRange(VkImageAspectFlags aspectMask);
    };

};
