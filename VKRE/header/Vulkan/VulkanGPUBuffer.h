#pragma once

#include <Vulkan/VulkanUtils.h>
#include <Vulkan/VulkanContext.h>

namespace VKRE {

    struct VulkanGPUBufferData {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation{};
        VmaAllocationInfo info{};
        uint64_t deviceAddress = 0;
    };

    namespace GPUBufferUtils {
        VulkanGPUBufferData ReCreateBuffer(VulkanContext& context, VulkanGPUBufferData* oldGPUBufferData, uint64_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateInfo& info);
        VulkanGPUBufferData CreateBuffer(VulkanContext& context, uint64_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateInfo& info);
        void ReleaseBuffer(VulkanContext& context, VulkanGPUBufferData* bufferData);
    }

}
