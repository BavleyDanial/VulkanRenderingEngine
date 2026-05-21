#include <Vulkan/VulkanPhysicalDevice.h>
#include <Vulkan/VulkanContext.h>

#include <unordered_set>
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace VKRE {

    namespace details {

        void combineFeatures(VkPhysicalDeviceFeatures& dest, VkPhysicalDeviceFeatures src) {
            dest.robustBufferAccess = dest.robustBufferAccess || src.robustBufferAccess;
            dest.fullDrawIndexUint32 = dest.fullDrawIndexUint32 || src.fullDrawIndexUint32;
            dest.imageCubeArray = dest.imageCubeArray || src.imageCubeArray;
            dest.independentBlend = dest.independentBlend || src.independentBlend;
            dest.geometryShader = dest.geometryShader || src.geometryShader;
            dest.tessellationShader = dest.tessellationShader || src.tessellationShader;
            dest.sampleRateShading = dest.sampleRateShading || src.sampleRateShading;
            dest.dualSrcBlend = dest.dualSrcBlend || src.dualSrcBlend;
            dest.logicOp = dest.logicOp || src.logicOp;
            dest.multiDrawIndirect = dest.multiDrawIndirect || src.multiDrawIndirect;
            dest.drawIndirectFirstInstance = dest.drawIndirectFirstInstance || src.drawIndirectFirstInstance;
            dest.depthClamp = dest.depthClamp || src.depthClamp;
            dest.depthBiasClamp = dest.depthBiasClamp || src.depthBiasClamp;
            dest.fillModeNonSolid = dest.fillModeNonSolid || src.fillModeNonSolid;
            dest.depthBounds = dest.depthBounds || src.depthBounds;
            dest.wideLines = dest.wideLines || src.wideLines;
            dest.largePoints = dest.largePoints || src.largePoints;
            dest.alphaToOne = dest.alphaToOne || src.alphaToOne;
            dest.multiViewport = dest.multiViewport || src.multiViewport;
            dest.samplerAnisotropy = dest.samplerAnisotropy || src.samplerAnisotropy;
            dest.textureCompressionETC2 = dest.textureCompressionETC2 || src.textureCompressionETC2;
            dest.textureCompressionASTC_LDR = dest.textureCompressionASTC_LDR || src.textureCompressionASTC_LDR;
            dest.textureCompressionBC = dest.textureCompressionBC || src.textureCompressionBC;
            dest.occlusionQueryPrecise = dest.occlusionQueryPrecise || src.occlusionQueryPrecise;
            dest.pipelineStatisticsQuery = dest.pipelineStatisticsQuery || src.pipelineStatisticsQuery;
            dest.vertexPipelineStoresAndAtomics = dest.vertexPipelineStoresAndAtomics || src.vertexPipelineStoresAndAtomics;
            dest.fragmentStoresAndAtomics = dest.fragmentStoresAndAtomics || src.fragmentStoresAndAtomics;
            dest.shaderTessellationAndGeometryPointSize = dest.shaderTessellationAndGeometryPointSize || src.shaderTessellationAndGeometryPointSize;
            dest.shaderImageGatherExtended = dest.shaderImageGatherExtended || src.shaderImageGatherExtended;
            dest.shaderStorageImageExtendedFormats = dest.shaderStorageImageExtendedFormats || src.shaderStorageImageExtendedFormats;
            dest.shaderStorageImageMultisample = dest.shaderStorageImageMultisample || src.shaderStorageImageMultisample;
            dest.shaderStorageImageReadWithoutFormat = dest.shaderStorageImageReadWithoutFormat || src.shaderStorageImageReadWithoutFormat;
            dest.shaderStorageImageWriteWithoutFormat = dest.shaderStorageImageWriteWithoutFormat || src.shaderStorageImageWriteWithoutFormat;
            dest.shaderUniformBufferArrayDynamicIndexing = dest.shaderUniformBufferArrayDynamicIndexing || src.shaderUniformBufferArrayDynamicIndexing;
            dest.shaderSampledImageArrayDynamicIndexing = dest.shaderSampledImageArrayDynamicIndexing || src.shaderSampledImageArrayDynamicIndexing;
            dest.shaderStorageBufferArrayDynamicIndexing = dest.shaderStorageBufferArrayDynamicIndexing || src.shaderStorageBufferArrayDynamicIndexing;
            dest.shaderStorageImageArrayDynamicIndexing = dest.shaderStorageImageArrayDynamicIndexing || src.shaderStorageImageArrayDynamicIndexing;
            dest.shaderClipDistance = dest.shaderClipDistance || src.shaderClipDistance;
            dest.shaderCullDistance = dest.shaderCullDistance || src.shaderCullDistance;
            dest.shaderFloat64 = dest.shaderFloat64 || src.shaderFloat64;
            dest.shaderInt64 = dest.shaderInt64 || src.shaderInt64;
            dest.shaderInt16 = dest.shaderInt16 || src.shaderInt16;
            dest.shaderResourceResidency = dest.shaderResourceResidency || src.shaderResourceResidency;
            dest.shaderResourceMinLod = dest.shaderResourceMinLod || src.shaderResourceMinLod;
            dest.sparseBinding = dest.sparseBinding || src.sparseBinding;
            dest.sparseResidencyBuffer = dest.sparseResidencyBuffer || src.sparseResidencyBuffer;
            dest.sparseResidencyImage2D = dest.sparseResidencyImage2D || src.sparseResidencyImage2D;
            dest.sparseResidencyImage3D = dest.sparseResidencyImage3D || src.sparseResidencyImage3D;
            dest.sparseResidency2Samples = dest.sparseResidency2Samples || src.sparseResidency2Samples;
            dest.sparseResidency4Samples = dest.sparseResidency4Samples || src.sparseResidency4Samples;
            dest.sparseResidency8Samples = dest.sparseResidency8Samples || src.sparseResidency8Samples;
            dest.sparseResidency16Samples = dest.sparseResidency16Samples || src.sparseResidency16Samples;
            dest.sparseResidencyAliased = dest.sparseResidencyAliased || src.sparseResidencyAliased;
            dest.variableMultisampleRate = dest.variableMultisampleRate || src.variableMultisampleRate;
            dest.inheritedQueries = dest.inheritedQueries || src.inheritedQueries;
        }

        bool supportsFeatures(const VkPhysicalDeviceFeatures& supported,
                const VkPhysicalDeviceFeatures& requested,
                const GenericFeatureChain& extensionSupported,
                const GenericFeatureChain& extensionRequested) {

            if (requested.robustBufferAccess && !supported.robustBufferAccess) return false;
            if (requested.fullDrawIndexUint32 && !supported.fullDrawIndexUint32) return false;
            if (requested.imageCubeArray && !supported.imageCubeArray) return false;
            if (requested.independentBlend && !supported.independentBlend) return false;
            if (requested.geometryShader && !supported.geometryShader) return false;
            if (requested.tessellationShader && !supported.tessellationShader) return false;
            if (requested.sampleRateShading && !supported.sampleRateShading) return false;
            if (requested.dualSrcBlend && !supported.dualSrcBlend) return false;
            if (requested.logicOp && !supported.logicOp) return false;
            if (requested.multiDrawIndirect && !supported.multiDrawIndirect) return false;
            if (requested.drawIndirectFirstInstance && !supported.drawIndirectFirstInstance) return false;
            if (requested.depthClamp && !supported.depthClamp) return false;
            if (requested.depthBiasClamp && !supported.depthBiasClamp) return false;
            if (requested.fillModeNonSolid && !supported.fillModeNonSolid) return false;
            if (requested.depthBounds && !supported.depthBounds) return false;
            if (requested.wideLines && !supported.wideLines) return false;
            if (requested.largePoints && !supported.largePoints) return false;
            if (requested.alphaToOne && !supported.alphaToOne) return false;
            if (requested.multiViewport && !supported.multiViewport) return false;
            if (requested.samplerAnisotropy && !supported.samplerAnisotropy) return false;
            if (requested.textureCompressionETC2 && !supported.textureCompressionETC2) return false;
            if (requested.textureCompressionASTC_LDR && !supported.textureCompressionASTC_LDR) return false;
            if (requested.textureCompressionBC && !supported.textureCompressionBC) return false;
            if (requested.occlusionQueryPrecise && !supported.occlusionQueryPrecise) return false;
            if (requested.pipelineStatisticsQuery && !supported.pipelineStatisticsQuery) return false;
            if (requested.vertexPipelineStoresAndAtomics && !supported.vertexPipelineStoresAndAtomics) return false;
            if (requested.fragmentStoresAndAtomics && !supported.fragmentStoresAndAtomics) return false;
            if (requested.shaderTessellationAndGeometryPointSize && !supported.shaderTessellationAndGeometryPointSize) return false;
            if (requested.shaderImageGatherExtended && !supported.shaderImageGatherExtended) return false;
            if (requested.shaderStorageImageExtendedFormats && !supported.shaderStorageImageExtendedFormats) return false;
            if (requested.shaderStorageImageMultisample && !supported.shaderStorageImageMultisample) return false;
            if (requested.shaderStorageImageReadWithoutFormat && !supported.shaderStorageImageReadWithoutFormat) return false;
            if (requested.shaderStorageImageWriteWithoutFormat && !supported.shaderStorageImageWriteWithoutFormat) return false;
            if (requested.shaderUniformBufferArrayDynamicIndexing && !supported.shaderUniformBufferArrayDynamicIndexing) return false;
            if (requested.shaderSampledImageArrayDynamicIndexing && !supported.shaderSampledImageArrayDynamicIndexing) return false;
            if (requested.shaderStorageBufferArrayDynamicIndexing && !supported.shaderStorageBufferArrayDynamicIndexing) return false;
            if (requested.shaderStorageImageArrayDynamicIndexing && !supported.shaderStorageImageArrayDynamicIndexing) return false;
            if (requested.shaderClipDistance && !supported.shaderClipDistance) return false;
            if (requested.shaderCullDistance && !supported.shaderCullDistance) return false;
            if (requested.shaderFloat64 && !supported.shaderFloat64) return false;
            if (requested.shaderInt64 && !supported.shaderInt64) return false;
            if (requested.shaderInt16 && !supported.shaderInt16) return false;
            if (requested.shaderResourceResidency && !supported.shaderResourceResidency) return false;
            if (requested.shaderResourceMinLod && !supported.shaderResourceMinLod) return false;
            if (requested.sparseBinding && !supported.sparseBinding) return false;
            if (requested.sparseResidencyBuffer && !supported.sparseResidencyBuffer) return false;
            if (requested.sparseResidencyImage2D && !supported.sparseResidencyImage2D) return false;
            if (requested.sparseResidencyImage3D && !supported.sparseResidencyImage3D) return false;
            if (requested.sparseResidency2Samples && !supported.sparseResidency2Samples) return false;
            if (requested.sparseResidency4Samples && !supported.sparseResidency4Samples) return false;
            if (requested.sparseResidency8Samples && !supported.sparseResidency8Samples) return false;
            if (requested.sparseResidency16Samples && !supported.sparseResidency16Samples) return false;
            if (requested.sparseResidencyAliased && !supported.sparseResidencyAliased) return false;
            if (requested.variableMultisampleRate && !supported.variableMultisampleRate) return false;
            if (requested.inheritedQueries && !supported.inheritedQueries) return false;

            return extensionSupported.MatchAll(extensionRequested);
        }

        GenericFeaturesPNextNode::GenericFeaturesPNextNode() { DisableFields(); }

        void GenericFeaturesPNextNode::DisableFields() {
            for (auto& field : fields) {
                field = VK_FALSE;
            }
        }

        bool GenericFeaturesPNextNode::Match(const GenericFeaturesPNextNode& requested, const GenericFeaturesPNextNode& supported) noexcept {
            assert(requested.sType == supported.sType && "Non-matching sTypes in features nodes!");
            for (uint32_t i = 0; i < fieldCapacity; i++) {
                if (requested.fields[i] && !supported.fields[i])
                    return false;
            }
            return true;
        }

        void GenericFeaturesPNextNode::Combine(const GenericFeaturesPNextNode& right) noexcept {
            assert(sType == right.sType && "Non-matching sTypes in features nodes!");
            for (uint32_t i = 0; i < GenericFeaturesPNextNode::fieldCapacity; i++) {
                fields[i] = fields[i] || right.fields[i];
            }
        }

        bool GenericFeatureChain::MatchAll(const GenericFeatureChain& extensionRequested) const noexcept {
            if (extensionRequested.nodes.size() != nodes.size()) {
                return false;
            }

            for (size_t i = 0; i < extensionRequested.nodes.size() && i < nodes.size(); ++i) {
                if (!GenericFeaturesPNextNode::Match(extensionRequested.nodes[i], nodes[i]))
                    return false;
            }
            return true;
        }

        bool GenericFeatureChain::FindAndMatch(const GenericFeatureChain& extensionsRequested) const noexcept {
            for (const auto& requestedExtensionNode : extensionsRequested.nodes) {
                bool found = false;
                for (const auto& supported_node : nodes) {
                    if (supported_node.sType == requestedExtensionNode.sType) {
                        found = true;
                        if (!GenericFeaturesPNextNode::Match(requestedExtensionNode, supported_node))
                            return false;
                        break;
                    }
                }
                if (!found)
                    return false;
            }
            return true;
        }

        void GenericFeatureChain::ChainUp(VkPhysicalDeviceFeatures2& features) noexcept {
            GenericFeaturesPNextNode* prev = nullptr;
            for (auto& extension : nodes) {
                if (prev != nullptr) {
                    prev->pNext = &extension;
                }
                prev = &extension;
            }
            features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features.pNext = !nodes.empty() ? &nodes.at(0) : nullptr;
        }

        void GenericFeatureChain::Combine(const GenericFeatureChain& right) noexcept {
            for (const auto& rightNode : right.nodes) {
                bool alreadyContained = false;
                for (auto& leftNode : nodes) {
                    if (leftNode.sType == rightNode.sType) {
                        leftNode.Combine(rightNode);
                        alreadyContained = true;
                    }
                }
                if (!alreadyContained) {
                    nodes.push_back(rightNode);
                }
            }
        }
    }

    VulkanPhysicalDeviceSelector::VulkanPhysicalDeviceSelector(VkInstance instance)
        :VulkanPhysicalDeviceSelector(instance, nullptr) {}

    VulkanPhysicalDeviceSelector::VulkanPhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface)
        :mInstance(instance), mSurface(surface) {
        SetPreferredType();
    }

    std::optional<VulkanPhysicalDevice> VulkanPhysicalDeviceSelector::Select() {
        std::vector<VulkanPhysicalDevice> devices = GetSuitableDevices();
        if (devices.empty()) {
            return std::nullopt;
        }

        return devices[0];
    }

    VulkanPhysicalDevice VulkanPhysicalDeviceSelector::PopulatePhysicalDevice(VkPhysicalDevice vkPhysicalDevice) const {
        VulkanPhysicalDevice physicalDevice{};
        physicalDevice.handle = vkPhysicalDevice;
        physicalDevice.name = physicalDevice.properties.deviceName;
        physicalDevice.surface = mSurface;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
        physicalDevice.queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, physicalDevice.queueFamilies.data());
        physicalDevice.queueFamilyIndicies = FindQueueFamilies(physicalDevice.handle);

        vkGetPhysicalDeviceProperties(physicalDevice.handle, &physicalDevice.properties);
        vkGetPhysicalDeviceFeatures(physicalDevice.handle, &physicalDevice.enabledFeatures.features);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice.handle, &physicalDevice.memoryProperties);

        uint32_t availableExtensionsCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice.handle, nullptr, &availableExtensionsCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(availableExtensionsCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice.handle, nullptr, &availableExtensionsCount, availableExtensions.data());

        for (const auto& extension : availableExtensions) {
            physicalDevice.availableExtensions.push_back(&extension.extensionName[0]);
        }

        details::GenericFeatureChain fillChain = mRequiredExtendedFeaturesChain;

        if (!fillChain.nodes.empty()) {
            VkPhysicalDeviceFeatures2 localFeatures{};
            fillChain.ChainUp(localFeatures);
            vkGetPhysicalDeviceFeatures2(physicalDevice.handle, &localFeatures);
            physicalDevice.extendedFeaturesChain = fillChain;
        }

        return physicalDevice;
    }

    std::vector<VulkanPhysicalDevice> VulkanPhysicalDeviceSelector::GetSuitableDevices() {
        uint32_t availableDevicesCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &availableDevicesCount, nullptr);
        std::vector<VkPhysicalDevice> availablePhysicalDevices(availableDevicesCount);
        vkEnumeratePhysicalDevices(mInstance, &availableDevicesCount, availablePhysicalDevices.data());

        if (availableDevicesCount == 0) {
            return {};
        }

        auto FillPhysicalDeviceWithCriteria = [&](VulkanPhysicalDevice& physicalDevice) {
            physicalDevice.enabledFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            mRequiredExtendedFeaturesChain.ChainUp(physicalDevice.enabledFeatures);
            physicalDevice.enabledFeatures.features = mRequiredFeatures;
            physicalDevice.extendedFeaturesChain = mRequiredExtendedFeaturesChain;
            physicalDevice.extensionsEnabled.append_range(mRequiredExtensions);
        };

        std::vector<VulkanPhysicalDevice> physicalDevices;
        for (auto& vkPhysicalDevice : availablePhysicalDevices) {
            VulkanPhysicalDevice physicalDevice = PopulatePhysicalDevice(vkPhysicalDevice);
            physicalDevice.isSuitable = IsSuitable(physicalDevice);
            if (physicalDevice.isSuitable) {
                physicalDevices.push_back(physicalDevice);
            }
        }

        for (auto& physicalDevice : physicalDevices)
            FillPhysicalDeviceWithCriteria(physicalDevice);

        return physicalDevices;
    }

    bool VulkanPhysicalDeviceSelector::IsSuitable(const VulkanPhysicalDevice& device) const {
        QueueFamilyIndinces indices = FindQueueFamilies(device.handle);
        if (!indices.IsComplete())
            return false;

        std::unordered_set<std::string> requiredExtensionSupport;
        requiredExtensionSupport.insert_range(mRequiredExtensions);

        uint32_t availableExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device.handle, nullptr, &availableExtensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
        vkEnumerateDeviceExtensionProperties(device.handle, nullptr, &availableExtensionCount, availableExtensions.data());

        for (const auto& extension : availableExtensions) {
            requiredExtensionSupport.erase(extension.extensionName);
        }

        if (!requiredExtensionSupport.empty())
            return false;

        if (device.properties.deviceType != mRequiredProperties.deviceType) {
            // TODO: return a semi-compatible value
        }

        bool supportsAllFeatures = details::supportsFeatures(device.enabledFeatures.features, mRequiredFeatures, device.extendedFeaturesChain, mRequiredExtendedFeaturesChain);
        if (!supportsAllFeatures)
            return false;

        return true;
    }

    QueueFamilyIndinces VulkanPhysicalDeviceSelector::FindQueueFamilies(VkPhysicalDevice device) const {
        QueueFamilyIndinces indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int index = 0;
        for (const auto& queue : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, index, mSurface, &presentSupport);

            if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT && !indices.graphicsFamily.has_value()) {
                indices.graphicsFamily = index;
            }

            if (presentSupport && !indices.presentFamily.has_value()) {
                indices.presentFamily = index;
            }

            index++;
            if (indices.IsComplete())
                break;
        }

        return indices;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetName(std::string_view name) {
        mName = name;
        return *this;
    }

    // TODO: Change this completely because this is stupid
    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetPreferredType(PhysicalDeviceType type) {
        switch (type) {
         case PhysicalDeviceType::INTEGRATED:
             mRequiredProperties.deviceType = VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
             break;
         case PhysicalDeviceType::DEDICATED:
             mRequiredProperties.deviceType = VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
             break;
        }

        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetRequiredQueueFamilies(const std::vector<uint32_t>& queueFlags) {
        for (const uint32_t queue : queueFlags) {
            mRequiredQueueFamilies |= queue;
        }
        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetRequiredExtensions(const std::vector<const char*>& extensions) {
        mRequiredExtensions.append_range(extensions);
        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetRequiredFeatures(const VkPhysicalDeviceFeatures& features) {
        details::combineFeatures(mRequiredFeatures, features);
        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetRequiredFeatures11(const VkPhysicalDeviceVulkan11Features& features) {
        VkPhysicalDeviceVulkan11Features featuresCopy = features;
        featuresCopy.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        AddRequiredExtensionFeatures(featuresCopy);
        return *this;
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetRequiredFeatures12(const VkPhysicalDeviceVulkan12Features& features) {
        VkPhysicalDeviceVulkan12Features featuresCopy = features;
        featuresCopy.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        return AddRequiredExtensionFeatures(featuresCopy);
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetRequiredFeatures13(const VkPhysicalDeviceVulkan13Features& features) {
        VkPhysicalDeviceVulkan13Features featuresCopy = features;
        featuresCopy.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        return AddRequiredExtensionFeatures(featuresCopy);
    }

    VulkanPhysicalDeviceSelector& VulkanPhysicalDeviceSelector::SetSurface(VkSurfaceKHR surface) {
        mSurface = surface;
        return *this;
    }

}
