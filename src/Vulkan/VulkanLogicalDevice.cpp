#include <Vulkan/VulkanLogicalDevice.h>

#include <unordered_set>
#include <cassert>

namespace VKRE {


    VulkanLogicalDeviceBuilder::VulkanLogicalDeviceBuilder(const VulkanPhysicalDevice& physicalDevice)
        :mPhysicalDevice(physicalDevice) {}

    std::optional<VulkanLogicalDevice> VulkanLogicalDeviceBuilder::Build() const {
        QueueFamilyIndinces queueIndices = mPhysicalDevice.queueFamilyIndicies;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::unordered_set<std::optional<uint32_t>> uniqueQueueFamilies = { queueIndices.graphicsFamily, queueIndices.presentFamily };

        float queuePriority = 1.0f;
        for (std::optional<uint32_t> queueFamily : uniqueQueueFamilies) {
            if (!queueFamily.has_value())
                break;

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily.value();
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);

        }

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &mPhysicalDevice.features;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(mPhysicalDevice.extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = mPhysicalDevice.extensions.data();

        VulkanLogicalDevice logicalDevice{};
        if (vkCreateDevice(mPhysicalDevice.handle, &deviceCreateInfo, nullptr, &logicalDevice.handle) != VK_SUCCESS) {
            assert("Couldn't create logical device!");
            return std::nullopt;
        }

        if (queueIndices.graphicsFamily.has_value())
            vkGetDeviceQueue(logicalDevice.handle, queueIndices.graphicsFamily.value(), 0, &logicalDevice.graphicsQueue);
        if (queueIndices.presentFamily.has_value())
            vkGetDeviceQueue(logicalDevice.handle, queueIndices.presentFamily.value(), 0, &logicalDevice.presentQueue);

        return logicalDevice;
    }

}
