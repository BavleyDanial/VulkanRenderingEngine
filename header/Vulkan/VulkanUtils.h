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
            std::deque<std::function<void()>> deletors;

            void PushDeleteFunc(std::function<void()>&& function) {
                deletors.push_back(function);
            }

            void Flush() {
                for (auto deletor : deletors) {
                    (deletor)();
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

    };
}
