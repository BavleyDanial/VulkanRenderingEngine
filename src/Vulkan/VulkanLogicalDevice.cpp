#include "Vulkan/VulkanPhysicalDevice.h"
#include <Vulkan/VulkanLogicalDevice.h>
#include <array>
#include <cassert>
#include <print>

namespace VKRE {


    VulkanLogicalDeviceBuilder::VulkanLogicalDeviceBuilder(const VulkanPhysicalDevice& physicalDevice)
        :mPhysicalDevice(physicalDevice) {}

    std::optional<VulkanLogicalDevice> VulkanLogicalDeviceBuilder::Build() const {
        QueueFamilyIndinces queueIndices = mPhysicalDevice.queueFamilyIndicies;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::array<uint32_t, 1> uniqueQueueFamilies = { queueIndices.graphicsFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            if (queueFamily < 0)
                break;

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);

        }

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &mPhysicalDevice.features;
        deviceCreateInfo.enabledExtensionCount = 0;

        VulkanLogicalDevice logicalDevice{};
        if (vkCreateDevice(mPhysicalDevice.physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice.device) != VK_SUCCESS) {
            assert("Couldn't create logical device!");
            return std::nullopt;
        }
        vkGetDeviceQueue(logicalDevice.device, queueIndices.graphicsFamily.value(), 0, &logicalDevice.graphicsQueue);

        return logicalDevice;
    }

}
