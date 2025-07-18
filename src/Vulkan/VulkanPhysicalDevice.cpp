#include <Vulkan/VulkanPhysicalDevice.h>

#include <Vulkan/VulkanContext.h>
#include <vulkan/vulkan_core.h>

#include <unordered_set>

namespace VKRE {

    VulkanPhysicalDeviceSelector::VulkanPhysicalDeviceSelector(VkInstance instance)
        :VulkanPhysicalDeviceSelector(instance, nullptr) {}

    VulkanPhysicalDeviceSelector::VulkanPhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface)
        :mInstance(instance), mSurface(surface) {
        SetPreferredType();
    }


    std::optional<VulkanPhysicalDevice> VulkanPhysicalDeviceSelector::Build() const {
        std::vector<VkPhysicalDevice> avaialbleDevices = GetSuitableDevices();

        if (avaialbleDevices.empty())
            return std::nullopt;

        VkPhysicalDevice physicalDevice = avaialbleDevices[0];

        VulkanPhysicalDevice selectedDevice{};
        selectedDevice.name = mName;
        selectedDevice.physicalDevice = physicalDevice;
        selectedDevice.surface = mSurface;

        vkGetPhysicalDeviceProperties(physicalDevice, &selectedDevice.properties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &selectedDevice.features);
        selectedDevice.extensions = mRequiredExtensions;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        selectedDevice.queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, selectedDevice.queueFamilies.data());
        selectedDevice.queueFamilyIndicies = FindQueueFamilies(physicalDevice);

        return selectedDevice;
    }

    std::vector<VkPhysicalDevice> VulkanPhysicalDeviceSelector::GetSuitableDevices() const {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            return {};
        }

        std::vector<VkPhysicalDevice> availableDevices(deviceCount);
        vkEnumeratePhysicalDevices(mInstance, &deviceCount, availableDevices.data());

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

        uint32_t availableExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data());

        std::unordered_set<std::string> reqExtensions;
        reqExtensions.insert_range(mRequiredExtensions);

        for (const auto& extension : availableExtensions) {
            reqExtensions.erase(extension.extensionName);
        }

        if (deviceProperties.deviceType != mRequiredProperties.deviceType) return false;
        if (!FindQueueFamilies(device).IsComplete()) return false;
        if (!reqExtensions.empty()) return false;

        return true;
    }

    QueueFamilyIndinces VulkanPhysicalDeviceSelector::FindQueueFamilies(VkPhysicalDevice device) const {
        QueueFamilyIndinces indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int index = 0;
        for (const auto& queue : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, index, mSurface, &presentSupport);

            if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT && !indices.graphicsFamily.has_value()) {
                indices.graphicsFamily = index;
            }
            
            if (presentSupport && !indices.presentFamily.has_value()) {
                indices.presentFamily = index;
            }

            index++;
            if (indices.IsComplete())
                break;
        }

        return indices;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetName(std::string_view name) {
        mName = name;
        return *this;
    }

    // TODO: Change this completely because this is stupid
    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetPreferredType(PhysicalDeviceType type) {
        switch (type) {
         case PhysicalDeviceType::INTEGRATED:
             mRequiredProperties.deviceType = VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
             break;
         case PhysicalDeviceType::DEDICATED:
             mRequiredProperties.deviceType = VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
             break;
        }

        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetRequiredQueueFamilies(const std::vector<uint32_t>& queueFlags) {
        for (const uint32_t queue : queueFlags) {
            mRequiredQueueFamilies |= queue;
        }
        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetRequiredExtensions(const std::vector<const char*>& extensions) {
        mRequiredExtensions.append_range(extensions);
        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetSurface(VkSurfaceKHR surface) {
        mSurface = surface;
        return *this;
    }

}
