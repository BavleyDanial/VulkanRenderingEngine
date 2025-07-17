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

        bool IsComplete() {
            return graphicsFamily.has_value();
        }
    };

    struct VulkanPhysicalDevice {
        std::string name;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceFeatures features{};
        std::vector<VkQueueFamilyProperties> queueFamilies;
        QueueFamilyIndinces queueFamilyIndicies{};
    };

    class VulkanPhysicalDeviceSelector {
    public:
        explicit VulkanPhysicalDeviceSelector(const class VulkanContext* const context);
        explicit VulkanPhysicalDeviceSelector(const class VulkanContext* const context, VkSurfaceKHR surface);

        std::optional<VulkanPhysicalDevice> Build() const;

        VulkanPhysicalDeviceSelector& SetName(std::string_view name);
        VulkanPhysicalDeviceSelector& SetPreferredType(PhysicalDeviceType type = PhysicalDeviceType::DEDICATED);
        VulkanPhysicalDeviceSelector& SetSurface(VkSurfaceKHR surface);
        VulkanPhysicalDeviceSelector& SetRequiredQueueFamilies(const std::vector<uint32_t>& queueFlags);

    private:
        std::vector<VkPhysicalDevice> GetSuitableDevices() const;
        bool IsSuitable(VkPhysicalDevice device) const;
        QueueFamilyIndinces FindQueueFamilies(VkPhysicalDevice device) const;

    private:
        struct InstanceInfo {
            std::string name = "";
            VkInstance instance = VK_NULL_HANDLE;
            VkSurfaceKHR surface = VK_NULL_HANDLE;
        } mInstanceInfo;

        struct SelectionCriteria {
            VkPhysicalDeviceProperties properties{};
            VkPhysicalDeviceFeatures features{};
            uint32_t queueFamilies = 0;
        } mSelectionCriteria;

    };

}
