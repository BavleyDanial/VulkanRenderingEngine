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

        auto it = mShaderModules.find(handle);
        bool alreadyUploaded = it != mShaderModules.end();

        ShaderColdData* cold = mResourceManager.GetShaderCold(handle);
        assert(cold && "VulkanResourceCache Shader is valid but GetShaderCold returned nullptr");

        if (alreadyUploaded && !cold->isDirty)
            return true;

        if (alreadyUploaded)
            DestroyShader(handle);

        return CreateShaderModule(handle, cold);
    }

    void VulkanResourceCache::DestroyShader(ShaderHandle handle) {
        auto it = mShaderModules.find(handle);
        if (it == mShaderModules.end()) return;
        DestroyShaderModule(handle);
    }

    VkShaderModule VulkanResourceCache::GetShaderModule(ShaderHandle handle) const {
        auto it = mShaderModules.find(handle);
        if (it == mShaderModules.end()) return nullptr;
        return it->second;
    }

    bool VulkanResourceCache::IsShaderUploaded(ShaderHandle handle) const {
        return mShaderModules.contains(handle);
    }

    void VulkanResourceCache::SyncDirtyShaders() {
        std::vector<ShaderHandle> dirty;
        for (auto& [handle, module] : mShaderModules) {
            const ShaderColdData* cold = mResourceManager.GetShaderCold(handle);
            if (cold && cold->isDirty)
                dirty.push_back(handle);
        }

        for (ShaderHandle handle : dirty)
            UploadShader(handle);
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

    bool VulkanResourceCache::CreateShaderModule(ShaderHandle handle, ShaderColdData* cold) {
        if (cold->byteCode.empty()) {
            std::println("VulkanResourceCache::CreateShaderModule Cannot create shader module, SPIRV is empty (index={})", static_cast<uint32_t>(handle.index));
            return false;
        }


        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.codeSize = cold->byteCode.size() * sizeof(uint32_t);
        createInfo.pCode = cold->byteCode.data();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(mContext.GetLogicalDevice().handle, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            std::println("VulkanResourceCache::CreateShaderModule Shader module has failed (index={})", static_cast<uint32_t>(handle.index));
            return false;
        }

        mShaderModules[handle] = shaderModule;
        assert(cold && "VulkanResourceCache Shader is valid but GetShaderCold returned nullptr");
        cold->isDirty = false;

        return true;
    }

    void VulkanResourceCache::DestroyShaderModule(ShaderHandle handle) {
        auto it = mShaderModules.find(handle);
        if (it == mShaderModules.end()) return;
        vkDestroyShaderModule(mContext.GetLogicalDevice().handle, it->second, nullptr);
        mShaderModules.erase(it);
    }

}
