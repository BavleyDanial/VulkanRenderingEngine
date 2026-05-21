#include <Vulkan/VulkanDescriptors.h>

namespace VKRE {

    void DescriptorAllocator::InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
        std::vector<VkDescriptorPoolSize> poolSizes;
        for (auto& poolRatio : poolRatios) {
            poolSizes.push_back({
                    .type = poolRatio.type,
                    .descriptorCount = static_cast<uint32_t>(poolRatio.ratio * maxSets)
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

    void DescriptorAllocator::ClearDescriptors(VkDevice device) {
        vkResetDescriptorPool(device, mPool, 0);
    }

    void DescriptorAllocator::DestroyPool(VkDevice device) {
        vkDestroyDescriptorPool(device, mPool, nullptr);
    }

    VkDescriptorSet DescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet descriptorSet;
        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

        return descriptorSet;
    }

    VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages, void* next, VkDescriptorSetLayoutCreateFlags flags) {
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

    void DescriptorLayoutBuilder::AddBinding(VkDescriptorType type, uint32_t bindingIdx) {
        VkDescriptorSetLayoutBinding newLayoutBinding{};
        newLayoutBinding.descriptorType = type;
        newLayoutBinding.binding = bindingIdx;
        newLayoutBinding.descriptorCount = 1;
        mBindings.push_back(newLayoutBinding);
    }

    void DescriptorLayoutBuilder::Clear() {
        mBindings.clear();
    }
}
