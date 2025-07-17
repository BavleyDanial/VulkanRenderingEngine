#pragma once

#include "VulkanPhysicalDevice.h"

#include <optional>

namespace VKRE {

    struct VulkanLogicalDevice {
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE; // TODO: Make this support more queues
        VkQueue presentQueue = VK_NULL_HANDLE; // TODO: Make this support more queues

        void Destroy() {
            vkDestroyDevice(device, nullptr);
        }
    };

    class VulkanLogicalDeviceBuilder {
    public:
        explicit VulkanLogicalDeviceBuilder(const VulkanPhysicalDevice& physicalDevice);
        std::optional<VulkanLogicalDevice> Build() const;

    private:
        VulkanPhysicalDevice mPhysicalDevice;
    };

}
