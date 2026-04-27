#include <Vulkan/VulkanResourceCache.h>

#include <cassert>
#include <print>

namespace VKRE {

    VulkanResourceCache::VulkanResourceCache(VulkanContext& context, ResourceManager& manager)
        :mContext(context), mResourceManager(manager) {}

    VulkanResourceCache::~VulkanResourceCache() {
        DestroyAll();
    }

    bool VulkanResourceCache::UploadShader(ShaderHandle handle) {
        if (!mResourceManager.IsShaderValid(handle)) {
            std::println("VulkanResourceCache::UploadShader handle is invalid");
            return false;
        }

        const ShaderHotData* hot = mResourceManager.GetShaderHot(handle);
        assert(hot && "VulkanResourceCache Shader is valid but GetShaderHot returned nullptr");

        auto it = mShaderModules.find(handle.index);
        bool alreadyUploaded = it != mShaderModules.end();

        const ShaderColdData* cold = mResourceManager.GetShaderCold(handle);
        assert(cold && "VulkanResourceCache Shader is valid but GetShaderCold returned nullptr");

        if (alreadyUploaded && !cold->isDirty)
            return true;

        if (alreadyUploaded)
            DestroyShader(handle);

        return CreateShaderModule(handle, hot);
    }

    void VulkanResourceCache::DestroyShader(ShaderHandle handle) {
        auto it = mShaderModules.find(handle.index);
        if (it == mShaderModules.end()) return;
        DestroyShaderModule(handle.index);
    }

    VkShaderModule VulkanResourceCache::GetShaderModule(ShaderHandle handle) const {
        auto it = mShaderModules.find(handle.index);
        if (it == mShaderModules.end()) return nullptr;
        return it->second;
    }

    bool VulkanResourceCache::IsShaderUploaded(ShaderHandle handle) const {
        return mShaderModules.contains(handle.index);
    }

    void VulkanResourceCache::SyncDirtyShaders() {
        for (auto& [index, module] : mShaderModules) {
            ShaderHandle handle { index, 0 }; // generation doesn't matter

            const ShaderColdData* cold = mResourceManager.GetShaderCold(handle);
            if (cold && cold->isDirty)
                UploadShader(handle);
        }
    }

    void VulkanResourceCache::DestroyAllShaders() {
        VkDevice device = mContext.GetLogicalDevice().handle;
        for (auto& [index, module] : mShaderModules) {
            if (module != nullptr) vkDestroyShaderModule(device, module, nullptr);
        }
        mShaderModules.clear();
    }

    void VulkanResourceCache::DestroyAll() {
        DestroyAllShaders();
    }

    bool VulkanResourceCache::CreateShaderModule(ShaderHandle handle, const ShaderHotData* hot) {
        if (hot->byteCode.empty()) {
            std::println("VulkanResourceCache::CreateShaderModule Cannot create shader module, SPIRV is empty (index={})", static_cast<uint32_t>(handle.index));
            return false;
        }


        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.codeSize = hot->byteCode.size() * sizeof(uint32_t);
        createInfo.pCode = hot->byteCode.data();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(mContext.GetLogicalDevice().handle, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            std::println("VulkanResourceCache::CreateShaderModule Shader module has failed (index={})", static_cast<uint32_t>(handle.index));
            return false;
        }

        mShaderModules[handle.index] = shaderModule;
        ShaderColdData* cold = mResourceManager.GetShaderCold(handle);
        assert(cold && "VulkanResourceCache Shader is valid but GetShaderCold returned nullptr");
        cold->isDirty = false;

        return true;
    }

    void VulkanResourceCache::DestroyShaderModule(uint32_t index) {
        auto it = mShaderModules.find(index);
        if (it == mShaderModules.end()) return;
        vkDestroyShaderModule(mContext.GetLogicalDevice().handle, it->second, nullptr);
        mShaderModules.erase(it);
    }

}
