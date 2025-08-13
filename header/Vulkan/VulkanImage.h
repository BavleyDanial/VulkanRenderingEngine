#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace VKRE {

    struct ImageInfo {
        VkImage image;
        VkImageView imageView;
        VmaAllocation allocation;
        VkFormat format;
        VkExtent3D extent;
    };

    class VulkanImage2D {
    public:
        VulkanImage2D(std::shared_ptr<VulkanContext> context);
        ~VulkanImage2D();

        ImageInfo& GetImageInfo() { return mImageInfo; }

        void CreateImage(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, VkImageAspectFlags aspectFlags, VmaAllocationCreateInfo& info);
        void Release();

    private:
        std::shared_ptr<VulkanContext> mContext;
        ImageInfo mImageInfo;
    };


    namespace ImageUtils {
        void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
        void CopyImage(VkCommandBuffer cmd, VkImage src, VkImage dest, VkExtent2D srcSize, VkExtent2D dstSize);
        VkImageSubresourceRange ImageSubSourceRange(VkImageAspectFlags aspectMask);
    };

};
