#include <Vulkan/VulkanImage.h>

namespace VKRE {

    VulkanImage2D::VulkanImage2D(std::shared_ptr<VulkanContext> context)
        :mContext(context) {}

    VulkanImage2D::~VulkanImage2D() {
        Release();
    }

    void VulkanImage2D::CreateImage(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, VmaAllocationCreateInfo& allocInfo) {
        // TODO: First make sure that we have deleted the image

        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;

        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = format;
        info.extent = extent;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = usageFlags;

        VK_CHECK(vmaCreateImage(mContext->GetAllocator(), &info, &allocInfo, &mImageInfo.image, &mImageInfo.allocation, nullptr));
        mImageInfo.extent = extent;
        mImageInfo.format = format;

        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;

        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.image = mImageInfo.image;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;

        VK_CHECK(vkCreateImageView(mContext->GetLogicalDevice().handle, &imageViewCreateInfo, nullptr, &mImageInfo.imageView));
    }

    void VulkanImage2D::Release() {
        if (mImageInfo.image) {
            vmaDestroyImage(mContext->GetAllocator(), mImageInfo.image, mImageInfo.allocation);
        }

        if (mImageInfo.imageView) {
            vkDestroyImageView(mContext->GetLogicalDevice().handle, mImageInfo.imageView, nullptr);
        }
    }

    namespace ImageUtils {
        void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) {
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

        void CopyImage(VkCommandBuffer cmd, VkImage src, VkImage dest, VkExtent2D srcSize, VkExtent2D dstSize) {
            VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

            blitRegion.srcOffsets[1].x = srcSize.width;
            blitRegion.srcOffsets[1].y = srcSize.height;
            blitRegion.srcOffsets[1].z = 1;

            blitRegion.dstOffsets[1].x = dstSize.width;
            blitRegion.dstOffsets[1].y = dstSize.height;
            blitRegion.dstOffsets[1].z = 1;

            blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitRegion.srcSubresource.baseArrayLayer = 0;
            blitRegion.srcSubresource.layerCount = 1;
            blitRegion.srcSubresource.mipLevel = 0;

            blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitRegion.dstSubresource.baseArrayLayer = 0;
            blitRegion.dstSubresource.layerCount = 1;
            blitRegion.dstSubresource.mipLevel = 0;

            VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
            blitInfo.dstImage = dest;
            blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            blitInfo.srcImage = src;
            blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            blitInfo.filter = VK_FILTER_LINEAR;
            blitInfo.regionCount = 1;
            blitInfo.pRegions = &blitRegion;

            vkCmdBlitImage2(cmd, &blitInfo);
        }

        VkImageSubresourceRange ImageSubSourceRange(VkImageAspectFlags aspectMask) {
            VkImageSubresourceRange subImage {};
            subImage.aspectMask = aspectMask;
            subImage.baseMipLevel = 0;
            subImage.levelCount = VK_REMAINING_MIP_LEVELS;
            subImage.baseArrayLayer = 0;
            subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

            return subImage;
        }
    }
}
