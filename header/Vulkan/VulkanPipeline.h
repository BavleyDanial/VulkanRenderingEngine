#pragma once

#include <vulkan/vulkan.h>
#include <ResourceManager/ResourceHandles.h>

#include <functional>

inline bool operator==(const VkPushConstantRange& a, const VkPushConstantRange& b) {
    return a.stageFlags == b.stageFlags && a.size == b.size && a.offset == b.offset;
}

namespace VKRE {

    struct VulkanPipelineLayoutKey {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts; // TODO: Change this from vector to something better for hashing
        std::vector<VkPushConstantRange> pushConstantRanges; // TODO: Change this from vector to something better for hashing

        bool operator==(const VulkanPipelineLayoutKey&) const = default;
        bool operator!=(const VulkanPipelineLayoutKey&) const = default;

        static VulkanPipelineLayoutKey Null() { return {}; }
    };

    struct VulkanComputePipelineKey {
        ShaderHandle shader;
        VkPipelineLayout layout = VK_NULL_HANDLE;

        bool operator==(const VulkanComputePipelineKey&) const = default;
        bool operator!=(const VulkanComputePipelineKey&) const = default;

        static VulkanComputePipelineKey Null() { return { ShaderHandle::Null(), VK_NULL_HANDLE }; }
    };

    struct VulkanComputePipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;

        bool Succeeded() const { return pipeline != VK_NULL_HANDLE; }

        void Destroy(VkDevice device) {
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, pipeline, nullptr);
            }
            pipeline = VK_NULL_HANDLE;
            layout = VK_NULL_HANDLE; // don't destroy this, but point to nullptr, let the cache destroy it
        }
    };

}

namespace std {
    template<>
    struct hash<VKRE::VulkanComputePipelineKey> {
        size_t operator()(const VKRE::VulkanComputePipelineKey& key) const noexcept {
            size_t result = std::hash<VKRE::ShaderHandle>{}(key.shader);
            result ^= std::hash<uint64_t>{}(reinterpret_cast<uint64_t>(key.layout)) + 0x9e3779b9 + (result << 6) + (result >> 2);
            return result;
        }
    };

    // TODO: This is horrible for performance
    template<>
    struct hash<VKRE::VulkanPipelineLayoutKey> {
        size_t operator()(const VKRE::VulkanPipelineLayoutKey& key) const noexcept {
            size_t result = 0;

            for (auto layout : key.descriptorSetLayouts)
                result ^= std::hash<uint64_t>{}(reinterpret_cast<uint64_t>(layout)) + 0x9e3779b9 + (result << 6) + (result >> 2);

            for (auto& range : key.pushConstantRanges) {
                result ^= std::hash<uint32_t>{}(range.size) + 0x9e3779b9 + (result << 6) + (result >> 2);
                result ^= std::hash<uint32_t>{}(range.offset) + 0x9e3779b9 + (result << 6) + (result >> 2);
                result ^= std::hash<uint32_t>{}(range.stageFlags) + 0x9e3779b9 + (result << 6) + (result >> 2);
            }

            return result;
        }
    };
}
