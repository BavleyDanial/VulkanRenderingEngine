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

    class VulkanGPUBuffer {
    public:
        VulkanGPUBuffer(VulkanContext& context);
        ~VulkanGPUBuffer();

        VulkanGPUBuffer(VulkanGPUBuffer&& other) noexcept;
        VulkanGPUBuffer& operator=(VulkanGPUBuffer&& other) noexcept;

        VulkanGPUBufferData& GetGPUBufferInfo() { return mGPUBufferData; }
        const VulkanGPUBufferData& GetGPUBufferInfo() const { return mGPUBufferData; }

        void ReCreateBuffer(uint64_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateInfo& info);
        void CreateBuffer(uint64_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateInfo& info);
        void Release();

    private:
        VulkanContext& mContext;
        VulkanGPUBufferData mGPUBufferData;

    };

}
