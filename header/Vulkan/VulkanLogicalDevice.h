#pragma once

#include "VulkanPhysicalDevice.h"

#include <optional>

namespace VKRE {

    struct VulkanLogicalDevice {
        VkDevice handle = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE; // TODO: Make this support more queues
        VkQueue presentQueue = VK_NULL_HANDLE; // TODO: Make this support more queues

        void Destroy() {
            vkDestroyDevice(handle, nullptr);
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
