#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <optional>
#include <vector>

namespace VKRE {

    enum class PhysicalDeviceType {
        INTEGRATED, DEDICATED
    };

    enum class PhysicalDeviceFeatureSet {
        ONE_ZERO, ONE_ONE, ONE_TWO, ONE_THREE
    };

    // TODO: Change this so that not all queues are required
    struct QueueFamilyIndinces {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct VulkanPhysicalDevice {
        std::string name;
        VkPhysicalDevice handle = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceFeatures features{};
        std::vector<const char*> extensions;

        std::vector<VkQueueFamilyProperties> queueFamilies;
        QueueFamilyIndinces queueFamilyIndicies{};
    };

    class VulkanPhysicalDeviceSelector {
    public:
        explicit VulkanPhysicalDeviceSelector(VkInstance instance);
        explicit VulkanPhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface);

        std::optional<VulkanPhysicalDevice> Build() const;

        VulkanPhysicalDeviceSelector& SetName(std::string_view name);
        VulkanPhysicalDeviceSelector& SetPreferredType(PhysicalDeviceType type = PhysicalDeviceType::DEDICATED);
        VulkanPhysicalDeviceSelector& SetSurface(VkSurfaceKHR surface);
        VulkanPhysicalDeviceSelector& SetRequiredQueueFamilies(const std::vector<uint32_t>& queueFlags);
        VulkanPhysicalDeviceSelector& SetRequiredExtensions(const std::vector<const char*>& extensions);

    private:
        std::vector<VkPhysicalDevice> GetSuitableDevices() const;
        bool IsSuitable(VkPhysicalDevice device) const;
        QueueFamilyIndinces FindQueueFamilies(VkPhysicalDevice device) const;

    private:
        std::string mName = "";
        VkInstance mInstance = VK_NULL_HANDLE;
        VkSurfaceKHR mSurface = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties mRequiredProperties{};
        VkPhysicalDeviceFeatures mRequiredFeatures{};
        std::vector<const char*> mRequiredExtensions;
        uint32_t mRequiredQueueFamilies = 0;
    };

}
