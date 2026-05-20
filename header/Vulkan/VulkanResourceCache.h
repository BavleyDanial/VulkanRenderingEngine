#pragma once

#include "VulkanContext.h"
#include "VulkanPipeline.h"

#include <ResourceManager/Resources.h>
#include <ResourceManager/ResourceHandles.h>
#include <ResourceManager/ResourceManager.h>

#include <unordered_map>
#include <vulkan/vulkan_core.h>

namespace VKRE {

    class VulkanResourceCache {
    public:
        VulkanResourceCache(VulkanContext& context, ResourceManager& manager);

        bool CreateShader(ShaderHandle handle);
        VkShaderModule GetShaderModule(ShaderHandle handle);
        VkShaderModule GetShaderModule(ShaderHandle handle) const;
        bool IsShaderUploaded(ShaderHandle handle) const;

        void SyncDirtyShaders();
        void DestroyShader(ShaderHandle handle);
        void DestroyAllShaders();

        VkPipelineLayout CreatePipelineLayout(const VulkanPipelineLayoutKey& key);
        VkPipelineLayout GetPipelineLayout(const VulkanPipelineLayoutKey& key);
        VkPipelineLayout GetPipelineLayout(const VulkanPipelineLayoutKey& key) const;
        bool IsPipelineLayoutCreated(const VulkanPipelineLayoutKey& key) const;

        void DestroyPipelineLayout(const VulkanPipelineLayoutKey& key);
        void DestroyAllPipelineLayouts();

        VulkanComputePipeline* CreateComputePipeline(const VulkanComputePipelineKey& key);
        VulkanComputePipeline* GetComputePipeline(const VulkanComputePipelineKey& key);
        const VulkanComputePipeline* GetComputePipeline(const VulkanComputePipelineKey& key) const;
        bool IsComputePipelineCreated(const VulkanComputePipelineKey& key) const;

        void DestroyComputePipeline(const VulkanComputePipelineKey& key);
        void DestroyAllComputePipelines();

        void DestroyAll();

    private:
        bool CreateShaderModule(ShaderHandle handle, ShaderColdData* cold);

    private:
        VulkanContext& mContext;
        ResourceManager& mResourceManager;

        std::unordered_map<ShaderHandle, VkShaderModule> mShaderModules;
        std::unordered_map<ShaderHandle, uint32_t> mShaderModulesRefCount;
        std::unordered_map<VulkanPipelineLayoutKey, VkPipelineLayout> mPipelineLayouts;
        std::unordered_map<VulkanComputePipelineKey, VulkanComputePipeline> mComputePipelines;
    };

}
