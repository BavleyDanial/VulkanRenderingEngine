#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vma/vk_mem_alloc.h>

#include <functional>
#include <deque>
#include <vector>
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

};
