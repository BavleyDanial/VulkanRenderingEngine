#include <Vulkan/VulkanGPUBuffer.h>

namespace VKRE {

    VulkanGPUBuffer::VulkanGPUBuffer(VulkanContext& context)
        :mContext(context) {}

    VulkanGPUBuffer::~VulkanGPUBuffer() {
        Release();
    }

    VulkanGPUBuffer::VulkanGPUBuffer(VulkanGPUBuffer&& other) noexcept
        : mContext(other.mContext), mGPUBufferData(other.mGPUBufferData) {
            other.mGPUBufferData = {};
        }

    VulkanGPUBuffer& VulkanGPUBuffer::operator=(VulkanGPUBuffer&& other) noexcept {
        if (this != &other) {
            Release();
            mGPUBufferData = other.mGPUBufferData;
            other.mGPUBufferData = {};
        }
        return *this;
    }

    void VulkanGPUBuffer::ReCreateBuffer(uint64_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateInfo& info) {
        Release();
        CreateBuffer(allocSize, usage, info);
    }

    void VulkanGPUBuffer::CreateBuffer(uint64_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateInfo& info) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.size = allocSize;
        bufferInfo.usage = usage;

        VK_CHECK(vmaCreateBuffer(mContext.GetAllocator(), &bufferInfo, &info,
                    &mGPUBufferData.buffer, &mGPUBufferData.allocation, &mGPUBufferData.info));

        if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            VkBufferDeviceAddressInfo deviceAddressInfo{};
            deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            deviceAddressInfo.buffer = mGPUBufferData.buffer;
            mGPUBufferData.deviceAddress = vkGetBufferDeviceAddress(mContext.GetLogicalDevice().handle, &deviceAddressInfo);
        } else {
            mGPUBufferData.deviceAddress = 0; 
        }
    }

    void VulkanGPUBuffer::Release() {
        if (mGPUBufferData.buffer == VK_NULL_HANDLE) return;
        vmaDestroyBuffer(mContext.GetAllocator(), mGPUBufferData.buffer, mGPUBufferData.allocation);
        mGPUBufferData = {};
    }

}
