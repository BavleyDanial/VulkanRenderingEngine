#pragma once

#include "VulkanUtils.h"

#include <span>
#include <vector>

namespace VKRE {

    class DescriptorAllocator {
    public:
        struct PoolSizeRatio {
            VkDescriptorType type;
            float ratio;
        };

    public:
        void InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
        void ClearDescriptors(VkDevice device);
        void DestroyPool(VkDevice device);

        VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);

    private:
        VkDescriptorPool mPool;
    };

    class DescriptorLayoutBuilder {
    public:
        void AddBinding(VkDescriptorType type, uint32_t bindingIdx);
        void Clear();

        VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages, void* next = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
    private:
        std::vector<VkDescriptorSetLayoutBinding> mBindings;
    };
}
