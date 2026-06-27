#pragma once

#include <Vulkan/VulkanUtils.h>

#include <deque>
#include <span>
#include <vector>

namespace VKRE {

    struct VulkanPoolSizeRatio {
        VkDescriptorType Type;
        float Ratio;
    };

    class VulkanFixedDescriptorAllocator {
    public:
        void InitPool(VkDevice device, uint32_t maxSets, std::span<VulkanPoolSizeRatio> poolRatios);
        void ClearPools(VkDevice device);
        void DestroyPool(VkDevice device);

        VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);

    private:
        VkDescriptorPool mPool;
    };

    class VulkanFixedBindlessDescriptorAllocator {
    public:
        void InitPool(VkDevice device, uint32_t maxImages);
        void DestroyPool(VkDevice device);

        int32_t RegisterImage(VkDevice device, VkDescriptorSet set, VkImageView imageView, VkSampler sampler);
        void UnRegisterImage(int32_t index);
        bool IsImagesFull() const { return mFreeList.empty() && mNextIndex >= mMaxImages; }

        VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);

    private:
        VkDescriptorPool mPool = VK_NULL_HANDLE;
        uint32_t mMaxImages = 0;
        int32_t mNextIndex = 0;
        std::vector<int32_t> mFreeList;
    };

    class VulkanGrowableDescriptorAllocator {
    public:
        void InitPool(VkDevice device, uint32_t initialSets, std::span<VulkanPoolSizeRatio> poolRatios);
        void ClearPools(VkDevice device);
        void DestroyPools(VkDevice device);

        VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext);

    private:
        VkDescriptorPool GetPool(VkDevice device);
        VkDescriptorPool CreatePool(VkDevice device, uint32_t setCount, std::span<VulkanPoolSizeRatio> poolRatios);

    private:
        std::vector<VulkanPoolSizeRatio> mPoolRatios;
        std::vector<VkDescriptorPool> mReadyPools;
        std::vector<VkDescriptorPool> mFullPools;
        uint32_t mSetsPerPool;
    };

    // TODO: Change this
    struct VulkanDescriptorWriter {
        std::deque<VkDescriptorImageInfo> ImageInfos;
        std::deque<VkDescriptorBufferInfo> BufferInfos;
        std::vector<VkWriteDescriptorSet> Writes;

        void WriteImage(int32_t binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
        void WriteBuffer(int32_t binding, VkBuffer buffer, uint64_t size, uint64_t offset, VkDescriptorType type);

        void Clear();
        void UpdateSet(VkDevice device, VkDescriptorSet set);
    };

    class VulkanDescriptorLayoutBuilder {
    public:
        void AddBinding(VkDescriptorType type, uint32_t bindingIdx, uint32_t descriptorCount = 1);
        void Clear();

        VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages, void* next = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
    private:
        std::vector<VkDescriptorSetLayoutBinding> mBindings;
    };
}
