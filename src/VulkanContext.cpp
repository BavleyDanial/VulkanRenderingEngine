#include <VulkanContext.h>

#include <Engine.h>
#include <vulkan/vulkan_core.h>

#include <print>
#include <cassert>
#include <cstring>

namespace VKRE {

    VulkanContext::VulkanContext() {
        if (mEnableValidationLayers && !CheckValidationLayerSupport()) {
            assert("Failed to create Vulkan Instance: Validation Layers are not supported!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "VK Rendering Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0 , 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_4;

        // TODO: Check if all required extensions are available
        std::vector<const char*> extensions = Engine::GetInstance()->GetWindow()->GetWindowExtensions();
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        if (mEnableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
            createInfo.ppEnabledLayerNames = mValidationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }


        if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
            assert("Failed to create Vulkan Instance!");
        }

        for (const char* extension : extensions) {
            std::println("{0}", extension);
        }
    }

    VulkanContext::~VulkanContext() {
        vkDestroyInstance(mInstance, nullptr);
    }

    bool VulkanContext::CheckValidationLayerSupport() {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : mValidationLayers) {
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

}
