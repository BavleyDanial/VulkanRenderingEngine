#include <Vulkan/VulkanDescriptors.h>

namespace VKRE {

    void VulkanFixedDescriptorAllocator::InitPool(VkDevice device, uint32_t maxSets, std::span<VulkanPoolSizeRatio> poolRatios) {
        std::vector<VkDescriptorPoolSize> poolSizes;
        for (auto& poolRatio : poolRatios) {
            poolSizes.push_back({
                .type = poolRatio.Type,
                .descriptorCount = static_cast<uint32_t>(poolRatio.Ratio * maxSets)
            });
        }

        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.maxSets = maxSets;
        createInfo.flags = 0;
        createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        createInfo.pPoolSizes = poolSizes.data();

        VK_CHECK(vkCreateDescriptorPool(device, &createInfo, nullptr, &mPool));
    }

    void VulkanFixedDescriptorAllocator::ClearPools(VkDevice device) {
        vkResetDescriptorPool(device, mPool, 0);
    }

    void VulkanFixedDescriptorAllocator::DestroyPool(VkDevice device) {
        vkDestroyDescriptorPool(device, mPool, nullptr);
    }

    VkDescriptorSet VulkanFixedDescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet descriptorSet;
        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

        return descriptorSet;
    }

    void VulkanFixedBindlessDescriptorAllocator::InitPool(VkDevice device, uint32_t maxImages) {
        mMaxImages = maxImages;

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = maxImages;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;

        VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &mPool));
    }

    void VulkanFixedBindlessDescriptorAllocator::DestroyPool(VkDevice device) {
        vkDestroyDescriptorPool(device, mPool, nullptr);
    }

    int32_t VulkanFixedBindlessDescriptorAllocator::RegisterImage(VkDevice device, VkDescriptorSet set, VkImageView imageView, VkSampler sampler) {
        if (!imageView) {
            std::println("VulkanBindlessDescriptorAllocator::RegisterTexture invalid image view");
            return -1;
        }

        if (IsImagesFull()) {
            std::println("VulkanBindlessDescriptorAllocator::RegisterTexture array is full");
            return -1;
        }

        int32_t index = 0;
        if (!mFreeList.empty()) {
            index = mFreeList.back();
            mFreeList.pop_back();
        } else {
            index = mNextIndex++;
        }

        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = sampler;
        imageInfo.imageView = imageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = 0;
        write.dstArrayElement = static_cast<uint32_t>(index);
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        return index;
    }

    void VulkanFixedBindlessDescriptorAllocator::UnRegisterImage(int32_t index) {
        if (index < 0 || index >= mNextIndex) {
            std::println("VulkanBindlessDescriptorAllocator::UnregisterTexture invalid index {}", index);
            return;
        }
        mFreeList.push_back(index);
    }

    VkDescriptorSet VulkanFixedBindlessDescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet set;
        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &set));
        return set;
    }

    void VulkanGrowableDescriptorAllocator::InitPool(VkDevice device, uint32_t initialSets, std::span<VulkanPoolSizeRatio> poolRatios) {
        mPoolRatios.clear();

        for (auto ratio : poolRatios)
            mPoolRatios.push_back(ratio);

        VkDescriptorPool newPool = CreatePool(device, initialSets, poolRatios);
        mSetsPerPool = initialSets * 1.5f;

        mReadyPools.push_back(newPool);
    }

    void VulkanGrowableDescriptorAllocator::ClearPools(VkDevice device) {
        for (auto pool : mReadyPools)
            vkResetDescriptorPool(device, pool, 0);

        for (auto pool : mFullPools) {
            vkResetDescriptorPool(device, pool, 0);
            mReadyPools.push_back(pool);
        }

        mFullPools.clear();
    }

    void VulkanGrowableDescriptorAllocator::DestroyPools(VkDevice device) {
        for (auto pool : mReadyPools)
            vkDestroyDescriptorPool(device, pool, nullptr);
        for (auto pool : mFullPools)
            vkDestroyDescriptorPool(device, pool, nullptr);

        mReadyPools.clear();
        mFullPools.clear();
    }

    VkDescriptorSet VulkanGrowableDescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext) {
        VkDescriptorPool poolToUse = GetPool(device);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = poolToUse;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;
        allocInfo.pNext = pNext;

        VkDescriptorSet descriptorSet;
        VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
            mFullPools.push_back(poolToUse);
            poolToUse = GetPool(device);
            allocInfo.descriptorPool = poolToUse;

            VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
        }

        mReadyPools.push_back(poolToUse);
        return descriptorSet;
    }

    VkDescriptorPool VulkanGrowableDescriptorAllocator::GetPool(VkDevice device) {
        VkDescriptorPool newPool;
        if (!mReadyPools.empty()) {
            newPool = mReadyPools.back();
            mReadyPools.pop_back();
        } else {
            newPool = CreatePool(device, mSetsPerPool, mPoolRatios);

            mSetsPerPool *= 1.5f;
            if (mSetsPerPool > 8192)
                mSetsPerPool = 8192;
        }

        return newPool;
    }

    VkDescriptorPool VulkanGrowableDescriptorAllocator::CreatePool(VkDevice device, uint32_t setCount, std::span<VulkanPoolSizeRatio> poolRatios) {
        std::vector<VkDescriptorPoolSize> poolSizes;
        for (auto& poolRatio : poolRatios) {
            poolSizes.push_back({
                .type = poolRatio.Type,
                .descriptorCount = static_cast<uint32_t>(poolRatio.Ratio * setCount)
            });
        }

        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.maxSets = setCount;
        createInfo.flags = 0;
        createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        createInfo.pPoolSizes = poolSizes.data();

        VkDescriptorPool newPool;
        VK_CHECK(vkCreateDescriptorPool(device, &createInfo, nullptr, &newPool));
        return newPool;
    }

    void VulkanDescriptorWriter::WriteImage(int32_t binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type) {
        VkDescriptorImageInfo& info = ImageInfos.emplace_back(VkDescriptorImageInfo{
            .sampler = sampler,
            .imageView = image,
            .imageLayout = layout
        });

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstBinding = binding;
        write.descriptorCount = 1;
        write.dstSet = VK_NULL_HANDLE;
        write.descriptorType = type;
        write.pImageInfo = &info;

        Writes.push_back(write);
    }

    void VulkanDescriptorWriter::WriteBuffer(int32_t binding, VkBuffer buffer, uint64_t size, uint64_t offset, VkDescriptorType type) {
        VkDescriptorBufferInfo& info = BufferInfos.emplace_back(VkDescriptorBufferInfo{
            .buffer = buffer,
            .offset = offset,
            .range = size
        });

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstBinding = binding;
        write.dstSet = VK_NULL_HANDLE;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pBufferInfo = &info;

        Writes.push_back(write);

    }

    void VulkanDescriptorWriter::Clear() {
        ImageInfos.clear();
        BufferInfos.clear();
        Writes.clear();
    }

    void VulkanDescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set) {
        for (auto& write : Writes)
            write.dstSet = set;
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(Writes.size()), Writes.data(), 0, nullptr);
    }

    VkDescriptorSetLayout VulkanDescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages, void* next, VkDescriptorSetLayoutCreateFlags flags) {
        for (auto& binding : mBindings) {
            binding.stageFlags |= shaderStages;
        }

        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.flags = flags;
        createInfo.bindingCount = static_cast<uint32_t>(mBindings.size());
        createInfo.pBindings = mBindings.data();
        createInfo.pNext = next;

        VkDescriptorSetLayout setLayout;
        VK_CHECK(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &setLayout));

        return setLayout;
    }

    void VulkanDescriptorLayoutBuilder::AddBinding(VkDescriptorType type, uint32_t bindingIdx, uint32_t descriptorCount) {
        VkDescriptorSetLayoutBinding newLayoutBinding{};
        newLayoutBinding.descriptorType = type;
        newLayoutBinding.binding = bindingIdx;
        newLayoutBinding.descriptorCount = descriptorCount;
        mBindings.push_back(newLayoutBinding);
    }

    void VulkanDescriptorLayoutBuilder::Clear() {
        mBindings.clear();
    }
}
