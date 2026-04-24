#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vma/vk_mem_alloc.h>

#include <functional>
#include <deque>
#include <vector>
#include <cstring>
#include <fstream>
#include <print>
#include <vulkan/vulkan_core.h>

#ifdef NDEBUG
#define VK_CHECK(x) x
#else
#define VK_CHECK(x)                                                         \
    do {                                                                    \
        if (x != VK_SUCCESS) {                                              \
            std::println("Vulkan Error Detected: {}", string_VkResult(x));  \
            abort();                                                        \
        }                                                                   \
    } while(false)                                                           // The do while loop only exists so that users of VK_CHECK() can end it with a ;
#endif

namespace VKRE {
    // TODO: Add debug fallback function
    class VulkanUtils {
    public:
        // TODO: Use templates for each Vulkan type that needs to be deleted and implement a custom deletor for each for performance
        struct DeletionQueue {
            std::vector<std::function<void()>> deletors;

            void PushDeleteFunc(std::function<void()>&& function) {
                deletors.push_back(function);
            }

            void Flush() {
                for (auto deletor = deletors.rbegin(); deletor != deletors.rend(); deletor++) {
                    (*deletor)();
                }

                deletors.clear();
            }
        };
    public:
        static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers) {
            uint32_t layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : validationLayers) {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        }

        // TODO: This should be in a shader system not in utils
        static VkShaderModule LoadShader(VkDevice device, const char* filePath) {
            // open the file. With cursor at the end
            std::ifstream file(filePath, std::ios::ate | std::ios::binary);

            if (!file.is_open()) {
                return VK_NULL_HANDLE;
            }

            // find what the size of the file is by looking up the location of the cursor
            // because the cursor is at the end, it gives the size directly in bytes
            size_t fileSize = (size_t)file.tellg();

            // spirv expects the buffer to be on uint32, so make sure to reserve a int
            // vector big enough for the entire file
            std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

            // put file cursor at beginning
            file.seekg(0);

            // load the entire file into the buffer
            file.read((char*)buffer.data(), fileSize);

            // now that the file is loaded into the buffer, we can close it
            file.close();

            // create a new shader module, using the buffer we loaded
            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.pNext = nullptr;

            // codeSize has to be in bytes, so multply the ints in the buffer by size of
            // int to know the real size of the buffer
            createInfo.codeSize = buffer.size() * sizeof(uint32_t);
            createInfo.pCode = buffer.data();

            // check that the creation goes well.
            VkShaderModule shaderModule;
            if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                return VK_NULL_HANDLE;
            }
            return shaderModule;
        }

        // TODO: This should be somewhere else maybe
        static VkRenderingAttachmentInfo AttatchmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout) {
            VkRenderingAttachmentInfo colorAttachment{};
            colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            colorAttachment.imageView = view;
            colorAttachment.imageLayout = layout;
            colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            if (clear) colorAttachment.clearValue = *clear;

            return colorAttachment;
        }

        // TODO: This should be somewhere else maybe
        static VkRenderingInfo RenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment) {
            VkRenderingInfo renderInfo {};
            renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
            renderInfo.pNext = nullptr;

            renderInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, renderExtent };
            renderInfo.layerCount = 1;
            renderInfo.colorAttachmentCount = 1;
            renderInfo.pColorAttachments = colorAttachment;
            renderInfo.pDepthAttachment = depthAttachment;
            renderInfo.pStencilAttachment = nullptr;

            return renderInfo;
        }
    };
}
