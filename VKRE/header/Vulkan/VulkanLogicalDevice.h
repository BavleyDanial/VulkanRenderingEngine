#pragma once

#include <Vulkan/VulkanUtils.h>
#include <Vulkan/VulkanPhysicalDevice.h>

#include <optional>
#include <unordered_map>

namespace VKRE {

    struct VulkanLogicalDevice {
        VkDevice handle = VK_NULL_HANDLE;
        std::unordered_map<uint32_t, VkQueue> queues;

        VkQueue GetQueue(QueueCapability queue) const {
            auto it = queues.find(static_cast<uint32_t>(queue));
            return it != queues.end() ? it->second : VK_NULL_HANDLE;
        };

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
