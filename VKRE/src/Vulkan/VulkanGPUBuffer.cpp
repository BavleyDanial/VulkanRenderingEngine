#include <Vulkan/VulkanGPUBuffer.h>

namespace VKRE {

    namespace GPUBufferUtils {

        VulkanGPUBufferData ReCreateBuffer(VulkanContext& context, VulkanGPUBufferData* oldGPUBufferData, uint64_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateInfo& info) {
            ReleaseBuffer(context, oldGPUBufferData);
            return CreateBuffer(context, allocSize, usage, info);
        }

        VulkanGPUBufferData CreateBuffer(VulkanContext& context, uint64_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateInfo& info) {
            VulkanGPUBufferData bufferData{};

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.pNext = nullptr;
            bufferInfo.size = allocSize;
            bufferInfo.usage = usage;

            VK_CHECK(vmaCreateBuffer(context.GetAllocator(), &bufferInfo, &info,
                        &bufferData.buffer, &bufferData.allocation, &bufferData.info));

            if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                VkBufferDeviceAddressInfo deviceAddressInfo{};
                deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                deviceAddressInfo.buffer = bufferData.buffer;
                bufferData.deviceAddress = vkGetBufferDeviceAddress(context.GetLogicalDevice().handle, &deviceAddressInfo);
            } else {
                bufferData.deviceAddress = 0; 
            }

            return bufferData;
        }

        void ReleaseBuffer(VulkanContext& context, VulkanGPUBufferData* bufferData) {
            if (bufferData->buffer == VK_NULL_HANDLE) return;
            vmaDestroyBuffer(context.GetAllocator(), bufferData->buffer, bufferData->allocation);
            *bufferData = {}; 
        }
    }

}
