#pragma once

#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace VKRE {

    class VulkanContext;
    class VulkanPhysicalDevice;
    class VulkanLogicalDevice;

    struct VulkanSwapChain {
        VkSwapchainKHR handle = VK_NULL_HANDLE;
        VkDevice deviceHandle = VK_NULL_HANDLE;
        VkSurfaceFormatKHR imageFormat{};
        VkPresentModeKHR presentMode{};
        VkExtent2D extent{};
        uint32_t minImageCount = 0;
        VkImageUsageFlags imageUsage;

        std::vector<VkImage> GetImages() const; // TODO: Error handling
        std::vector<VkImageView> GetImageViews(const std::vector<VkImage>& images) const; // TODO: Error handling

        void DestroyImageViews(std::vector<VkImageView>& imageViews);
        void Destroy(const VulkanLogicalDevice& logicalDevice) {
            vkDestroySwapchainKHR(logicalDevice.handle, handle, nullptr);
        }
    };

    class VulkanSwapChainBuilder {
    public:
        explicit VulkanSwapChainBuilder(VkInstance instance, VkSurfaceKHR surface, const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& logicalDevice);

        std::optional<VulkanSwapChain> Build();
        VulkanSwapChainBuilder& SetDesiredFormat(const VkSurfaceFormatKHR& format);
        VulkanSwapChainBuilder& SetDesiredPresentMode(const VkPresentModeKHR& mode);
        VulkanSwapChainBuilder& SetDesiredExtent(uint32_t width, uint32_t height);
        VulkanSwapChainBuilder& SetDesiredImageCount(uint32_t count);
        VulkanSwapChainBuilder& SetDesiredImageUsage(VkImageUsageFlags flag);

    private:
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };
    private:
         SwapChainSupportDetails QuerySupport();
         VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
         VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes) const;
         VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
         uint32_t ChooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities) const;

    private:
        VkInstance mInstance = VK_NULL_HANDLE;
        VkSurfaceKHR mSurface = VK_NULL_HANDLE;
        VulkanPhysicalDevice mPhysicalDevice{};
        VulkanLogicalDevice mLogicalDevice{};

        struct SwapChainConfig {
            VkSurfaceFormatKHR surfaceFormat{};
            VkPresentModeKHR presentMode{};
            VkFormat imageFormat{};
            VkExtent2D extent{};
            VkImageUsageFlags imageUsage;
            uint32_t imageCount = 0;
        } mSwapChainConfig;

    };

}
