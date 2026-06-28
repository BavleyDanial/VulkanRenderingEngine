#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vma/vk_mem_alloc.h>

#include <vector>
#include <cstring>
#include <print>

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

        // TODO: This should be somewhere else maybe
        static VkRenderingAttachmentInfo AttatchmentInfo(VkImageView view, VkClearValue* clear, VkAttachmentStoreOp storeOp, VkImageLayout layout) {
            VkRenderingAttachmentInfo colorAttachment{};
            colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            colorAttachment.imageView = view;
            colorAttachment.imageLayout = layout;
            colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachment.storeOp = storeOp;
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
