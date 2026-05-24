#pragma once

#include <Vulkan/VulkanUtils.h>

#include <string>
#include <vector>
#include <optional>

namespace VKRE {

    namespace details {

        struct GenericFeaturesPNextNode {

            static const uint32_t fieldCapacity = 256;

            GenericFeaturesPNextNode();

            void DisableFields();

            template <typename T> GenericFeaturesPNextNode(const T& features) noexcept {
                DisableFields();
                memcpy(this, &features, sizeof(T));
            }

            static bool Match(const GenericFeaturesPNextNode& requested, const GenericFeaturesPNextNode& supported) noexcept;
            void Combine(const GenericFeaturesPNextNode& right) noexcept;

            VkStructureType sType = static_cast<VkStructureType>(0);
            void* pNext = nullptr;
            VkBool32 fields[fieldCapacity];
        };

        struct GenericFeatureChain {
            std::vector<GenericFeaturesPNextNode> nodes;

            template <typename T> void Add(const T& features) noexcept {
                for (auto& node : nodes) {
                    if (static_cast<VkStructureType>(features.sType) == node.sType) {
                        node.Combine(features);
                        return;
                    }
                }
                nodes.push_back(features);
            }

            bool MatchAll(const GenericFeatureChain& extensionRequested) const noexcept;
            bool FindAndMatch(const GenericFeatureChain& extensionRequested) const noexcept;
            void ChainUp(VkPhysicalDeviceFeatures2& features) noexcept;
            void Combine(const GenericFeatureChain& right) noexcept;
        };
    }

    enum class PhysicalDeviceType {
        Integrated, Dedicated
    };

    enum class QueueCapability : uint32_t {
        Graphics = VK_QUEUE_GRAPHICS_BIT,
        Compute = VK_QUEUE_COMPUTE_BIT,
        Transfer = VK_QUEUE_TRANSFER_BIT,
        Present = 0x80000000,
    };

    struct QueueFamilyIndinces {
        std::unordered_map<uint32_t, uint32_t> families;

        bool Has(QueueCapability queue) const {
            return families.contains(static_cast<uint32_t>(queue));
        }

        uint32_t Get(QueueCapability queue) const {
            return families.at(static_cast<uint32_t>(queue));
        }

        bool IsComplete(uint32_t requiredQueues) const {
            uint32_t remaining = requiredQueues;
            while (remaining) {
                uint32_t lowestBit = remaining & (~remaining + 1);
                if (!families.contains(lowestBit))
                    return false;
                remaining &= remaining - 1;
            }
            return true;
        }
    };

    struct VulkanPhysicalDevice {
        std::string name;
        VkPhysicalDevice handle = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        bool isSuitable = false;

        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceFeatures2 enabledFeatures{};
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        details::GenericFeatureChain extendedFeaturesChain{};

        std::vector<const char*> extensionsEnabled;
        std::vector<std::string> availableExtensions;

        std::vector<VkQueueFamilyProperties> queueFamilies;
        QueueFamilyIndinces queueFamilyIndicies{};
    };

    class VulkanPhysicalDeviceSelector {
    public:
        explicit VulkanPhysicalDeviceSelector(VkInstance instance);
        explicit VulkanPhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface);

        std::optional<VulkanPhysicalDevice> Select();

        VulkanPhysicalDeviceSelector& SetName(std::string_view name);
        VulkanPhysicalDeviceSelector& SetPreferredType(PhysicalDeviceType type = PhysicalDeviceType::Dedicated);
        VulkanPhysicalDeviceSelector& SetSurface(VkSurfaceKHR surface);
        VulkanPhysicalDeviceSelector& SetRequiredQueueFamilies(const std::vector<QueueCapability>& queueFlags);
        VulkanPhysicalDeviceSelector& SetRequiredExtensions(const std::vector<const char*>& extensions);
        VulkanPhysicalDeviceSelector& SetRequiredFeatures(const VkPhysicalDeviceFeatures& features);
        VulkanPhysicalDeviceSelector& SetRequiredFeatures11(const VkPhysicalDeviceVulkan11Features& features);
        VulkanPhysicalDeviceSelector& SetRequiredFeatures12(const VkPhysicalDeviceVulkan12Features& features);
        VulkanPhysicalDeviceSelector& SetRequiredFeatures13(const VkPhysicalDeviceVulkan13Features& features);

    private:
        std::vector<VulkanPhysicalDevice> GetSuitableDevices();
        bool IsSuitable(const VulkanPhysicalDevice& device) const;
        QueueFamilyIndinces FindQueueFamilies(VkPhysicalDevice device) const;

        VulkanPhysicalDevice PopulatePhysicalDevice(VkPhysicalDevice vkPhysicalDevice) const;
        template <typename T> VulkanPhysicalDeviceSelector& AddRequiredExtensionFeatures(const T& features) {
            mRequiredExtendedFeaturesChain.Add(features);
            return *this;
        }

    private:
        std::string mName = "";
        VkInstance mInstance = VK_NULL_HANDLE;
        VkSurfaceKHR mSurface = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties mRequiredProperties{};
        VkPhysicalDeviceFeatures mRequiredFeatures{};
        details::GenericFeatureChain mRequiredExtendedFeaturesChain;

        std::vector<const char*> mRequiredExtensions;
        uint32_t mRequiredQueueFamilies = 0;
    };

}
