#pragma once

#include <Vulkan/VulkanUtils.h>
#include <Vulkan/VulkanContext.h>

#include <ResourceManager/Resources.h>
#include <ResourceManager/ResourceHandles.h>
#include <ResourceManager/ResourceManager.h>

#include <unordered_map>

namespace VKRE {

    class VulkanResourceCache {
    public:
        VulkanResourceCache(VulkanContext& context, ResourceManager& manager);
        ~VulkanResourceCache();

        bool UploadShader(ShaderHandle handle);
        void DestroyShader(ShaderHandle handle);

        VkShaderModule GetShaderModule(ShaderHandle handle) const;
        bool IsShaderUploaded(ShaderHandle handle) const;

        void SyncDirtyShaders();
        void DestroyAllShaders();

        void DestroyAll();
    private:
        bool CreateShaderModule(ShaderHandle handle, const ShaderHotData* hot);
        void DestroyShaderModule(uint32_t index);

    private:
        VulkanContext& mContext;
        ResourceManager& mResourceManager;

        std::unordered_map<uint32_t, VkShaderModule> mShaderModules;
    };

}
