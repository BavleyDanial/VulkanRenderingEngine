#pragma once

#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace VKRE {

    struct VulkanTextureData {
        VkImage image{};
        VkImageView imageView{};
        VmaAllocation allocation{};
        VkFormat format{};
        VkExtent3D extent{};
    };

    class VulkanTexture {
    public:
        VulkanTexture(VulkanContext* context);
        ~VulkanTexture();
        VulkanTexture(VulkanTexture&& other) noexcept;
        VulkanTexture& operator=(VulkanTexture&& other) noexcept;

        VulkanTextureData& GetTextureInfo() { return mTextureInfo; }
        const VulkanTextureData& GetTextureInfo() const { return mTextureInfo; }

        void ReCreateTexture(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, VmaAllocationCreateInfo& info);
        void CreateTexture(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, VmaAllocationCreateInfo& info);
        void Release();

    private:
        VulkanContext* mContext;
        VulkanTextureData mTextureInfo;
    };


    namespace ImageUtils {
        void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
        void CopyImage(VkCommandBuffer cmd, VkImage src, VkImage dest, VkExtent2D srcSize, VkExtent2D dstSize);
        VkImageSubresourceRange ImageSubSourceRange(VkImageAspectFlags aspectMask);
    };

};
