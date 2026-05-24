#pragma once

#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanGPUBuffer.h"

#include <ResourceManager/Resources.h>
#include <ResourceManager/ResourceHandles.h>
#include <ResourceManager/ResourceRefs.h>
#include <ResourceManager/ResourceManager.h>

#include <unordered_map>

namespace VKRE {

    class VulkanResourceCache {
    public:
        VulkanResourceCache(VulkanContext& context, ResourceManager& manager);

        bool AllocateBuffer(GPUBufferHandle handle);
        void UploadBuffer(GPUBufferHandle handle, const void* data, uint64_t size, uint64_t offset);
        VulkanGPUBufferData* GetBufferData(GPUBufferHandle handle);
        const VulkanGPUBufferData* GetBufferData(GPUBufferHandle handle) const;
        bool IsBufferAllocated(GPUBufferHandle handle) const;

        void DesroyBuffer(GPUBufferHandle handle);
        void DestroyAllBuffers();

        bool CreateShader(ShaderHandle handle);
        VkShaderModule GetShaderModule(ShaderHandle handle);
        VkShaderModule GetShaderModule(ShaderHandle handle) const;
        bool IsShaderUploaded(ShaderHandle handle) const;

        void SyncDirtyShaders();
        void DestroyAllShaders();

        VkPipelineLayout CreatePipelineLayout(const VulkanPipelineLayoutKey& key);
        VkPipelineLayout GetPipelineLayout(const VulkanPipelineLayoutKey& key);
        VkPipelineLayout GetPipelineLayout(const VulkanPipelineLayoutKey& key) const;
        bool IsPipelineLayoutCreated(const VulkanPipelineLayoutKey& key) const;

        void DestroyPipelineLayout(const VulkanPipelineLayoutKey& key);
        void DestroyAllPipelineLayouts();

        VulkanGraphicsPipeline* CreateGraphicsPipeline(const VulkanGraphicsPipelineKey& key);
        VulkanGraphicsPipeline* GetGraphicsPipeline(const VulkanGraphicsPipelineKey& key);
        const VulkanGraphicsPipeline* GetGraphicsPipeline(const VulkanGraphicsPipelineKey& key) const;
        bool IsGraphicsPipelineCreated(const VulkanGraphicsPipelineKey& key) const;

        void DestroyGraphicsPipeline(const VulkanGraphicsPipelineKey& key);
        void DestroyAllGraphicsPipelines();

        VulkanComputePipeline* CreateComputePipeline(const VulkanComputePipelineKey& key);
        VulkanComputePipeline* GetComputePipeline(const VulkanComputePipelineKey& key);
        const VulkanComputePipeline* GetComputePipeline(const VulkanComputePipelineKey& key) const;
        bool IsComputePipelineCreated(const VulkanComputePipelineKey& key) const;

        void DestroyComputePipeline(const VulkanComputePipelineKey& key);
        void DestroyAllComputePipelines();

        void DestroyAll();

    private:
        bool CreateShaderModule(ShaderHandle handle, ShaderColdData* cold);
        void DestroyShaderIfUnused(ShaderHandle handle);

    private:
        VulkanContext& mContext;
        ResourceManager& mResourceManager;

        std::unordered_map<GPUBufferHandle, VulkanGPUBuffer> mBuffers;
        std::unordered_map<ShaderHandle, VkShaderModule> mShaderModules;
        std::unordered_map<VulkanPipelineLayoutKey, VkPipelineLayout> mPipelineLayouts;
        std::unordered_map<VulkanComputePipelineKey, VulkanComputePipeline> mComputePipelines;
        std::unordered_map<VulkanGraphicsPipelineKey, VulkanGraphicsPipeline> mGraphicsPipelines;
    };

}
