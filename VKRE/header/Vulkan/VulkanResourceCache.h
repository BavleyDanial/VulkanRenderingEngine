#pragma once

#include <Vulkan/VulkanContext.h>

#include <Vulkan/VulkanGPUBuffer.h>
#include <Vulkan/VulkanPipeline.h>
#include <Vulkan/VulkanImage.h>

#include <ResourceManager/ResourceRefs.h>
#include <ResourceManager/ResourceHandles.h>
#include <ResourceManager/ResourceManager.h>

#include <unordered_map>

namespace VKRE {

    class VulkanResourceCache {
    public:
        VulkanResourceCache(VulkanContext& context, ResourceManager& manager);

        bool AllocateBuffer(GPUBufferHandle handle);
        VulkanGPUBufferData* GetBufferData(GPUBufferHandle handle);
        const VulkanGPUBufferData* GetBufferData(GPUBufferHandle handle) const;
        bool IsBufferAllocated(GPUBufferHandle handle) const;

        void DestroyBuffer(GPUBufferHandle handle);
        void DestroyAllBuffers();

        bool AllocateImage2D(Texture2DHandle handle);
        VulkanImageData* GetImageData2D(Texture2DHandle handle);
        const VulkanImageData* GetImageData2D(Texture2DHandle handle) const;
        bool IsImage2DAllocated(Texture2DHandle handle) const;

        void DestroyImage2D(Texture2DHandle handle);
        void DestroyAllImages2D();

        bool AllocateImageCube(TextureCubeHandle handle);
        VulkanImageData* GetImageDataCube(TextureCubeHandle handle);
        const VulkanImageData* GetImageDataCube(TextureCubeHandle handle) const;
        bool IsImageCubeAllocated(TextureCubeHandle handle) const;

        void DestroyImageCube(TextureCubeHandle handle);
        void DestroyAllImageCubes();

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

        std::vector<VulkanImageData> mImages2D;
        std::vector<VulkanImageData> mImagesCube;
        std::vector<VulkanGPUBufferData> mBuffers;

        std::unordered_map<ShaderHandle, VkShaderModule> mShaderModules;
        std::unordered_map<VulkanPipelineLayoutKey, VkPipelineLayout> mPipelineLayouts;
        std::unordered_map<VulkanComputePipelineKey, VulkanComputePipeline> mComputePipelines;
        std::unordered_map<VulkanGraphicsPipelineKey, VulkanGraphicsPipeline> mGraphicsPipelines;
    };

}
