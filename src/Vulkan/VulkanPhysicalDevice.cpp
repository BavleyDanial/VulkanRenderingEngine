#include <Vulkan/VulkanPhysicalDevice.h>

namespace VKRE {

    VulkanPhysicalDeviceSelector::VulkanPhysicalDeviceSelector(VkInstance instance)
        :VulkanPhysicalDeviceSelector(instance, VK_NULL_HANDLE) {}

    VulkanPhysicalDeviceSelector::VulkanPhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface) {
        mInstanceInfo.instance = instance;
        mInstanceInfo.surface = surface;
    }

    std::optional<VulkanPhysicalDevice> VulkanPhysicalDeviceSelector::Build() const {
        std::vector<VkPhysicalDevice> avaialbleDevices = GetSuitableDevices();

        if (avaialbleDevices.empty())
            return std::nullopt;

        VkPhysicalDevice physicalDevice = avaialbleDevices[0];

        VulkanPhysicalDevice selectedDevice{};
        selectedDevice.name = mInstanceInfo.name;
        selectedDevice.physicalDevice = physicalDevice;
        selectedDevice.surface = mInstanceInfo.surface;

        vkGetPhysicalDeviceProperties(physicalDevice, &selectedDevice.properties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &selectedDevice.features);

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        selectedDevice.queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, selectedDevice.queueFamilies.data());

        return selectedDevice;
    }

    std::vector<VkPhysicalDevice> VulkanPhysicalDeviceSelector::GetSuitableDevices() const {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(mInstanceInfo.instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            return {};
        }

        std::vector<VkPhysicalDevice> availableDevices(deviceCount);
        vkEnumeratePhysicalDevices(mInstanceInfo.instance, &deviceCount, availableDevices.data());

        std::vector<VkPhysicalDevice> selectedDevices;
        for (const auto device : availableDevices) {
            if (!IsSuitable(device)) {
                continue;
            }

            selectedDevices.push_back(device);
        }

        return selectedDevices;
    }

    // TODO: Find a better way to handle suitability check
    bool VulkanPhysicalDeviceSelector::IsSuitable(VkPhysicalDevice device) const {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        if (deviceProperties.deviceType == mSelectionCriteria.properties.deviceType) return true;
        if (FindQueueFamilies(device).IsComplete()) return true;

        return false;
    }

    QueueFamilyIndinces VulkanPhysicalDeviceSelector::FindQueueFamilies(VkPhysicalDevice device) const {
        QueueFamilyIndinces indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int index = 0;
        for (const auto& queue : queueFamilies) {
            if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsQueue = index;
                indices.presentQueue = index;
            }

            if (indices.IsComplete())
                break;
        }

        return indices;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetName(std::string_view name) {
        mInstanceInfo.name = name;
        return *this;
    }

    // TODO: Change this completely because this is stupid
    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetPreferredType(PhysicalDeviceType type) {
        switch (type) {
         case PhysicalDeviceType::INTEGRATED:
             mSelectionCriteria.properties.deviceType = VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
             break;
         case PhysicalDeviceType::DEDICATED:
             mSelectionCriteria.properties.deviceType = VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
             break;
        }

        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetRequiredQueueFamilies(const std::vector<uint32_t>& queueFlags) {
        for (const uint32_t queue : queueFlags) {
            mSelectionCriteria.queueFamilies |= queue;
        }
        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetSurface(VkSurfaceKHR surface) {
        mInstanceInfo.surface = surface;
        return *this;
    }

}
