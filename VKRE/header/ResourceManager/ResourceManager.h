#pragma once

#include "ResourceHandles.h"
#include "ResourcePool.h"
#include "Resources.h"

namespace VKRE {

    template<typename Tag> class ResourceRef;

    class ResourceManager {
    public:
        ResourceManager();
        ~ResourceManager();

        template<typename Tag>
        void AddRef(ResourceHandle<Tag> handle);
        template<typename Tag>
        void DestroyRef(ResourceHandle<Tag> handle);

        ResourceRef<ShaderTag> LoadShader(ShaderDesc& desc);

        ShaderHotData* GetShaderHot(ShaderHandle handle) { return mShaderPool.GetHot(handle); };
        const ShaderHotData* GetShaderHot(ShaderHandle handle) const { return mShaderPool.GetHot(handle); };
        ShaderColdData* GetShaderCold(ShaderHandle handle) { return mShaderPool.GetCold(handle); };
        const ShaderColdData* GetShaderCold(ShaderHandle handle) const { return mShaderPool.GetCold(handle); };

        void MarkShaderDirty(ShaderHandle handle);

        bool IsShaderValid(ShaderHandle handle) const { return mShaderPool.IsValid(handle); }
        uint32_t GetLiveShaderCount() const { return mShaderPool.GetLiveCount(); }

        ResourceRef<GPUBufferTag> CreateGPUBuffer(GPUBufferDesc& desc);

        GPUBufferHotData* GetGPUBufferHot(GPUBufferHandle handle) { return mGPUBufferPool.GetHot(handle); };
        const GPUBufferHotData* GetGPUBufferHot(GPUBufferHandle handle) const { return mGPUBufferPool.GetHot(handle); };
        GPUBufferColdData* GetGPUBufferCold(GPUBufferHandle handle) { return mGPUBufferPool.GetCold(handle); };
        const GPUBufferColdData* GetGPUBufferCold(GPUBufferHandle handle) const { return mGPUBufferPool.GetCold(handle); };

        bool IsGPUBufferValid(GPUBufferHandle handle) const { return mGPUBufferPool.IsValid(handle); }
        uint32_t GetLiveGPUBufferCount() const { return mGPUBufferPool.GetLiveCount(); }

    private:
        static constexpr uint32_t INITIAL_SHADER_POOL_CAP = 64;
        static constexpr uint32_t INITIAL_GPU_BUFFER_POOL_CAP = 64;
        static constexpr uint32_t INITIAL_MESH_POOL_CAP = 64;

        ResourcePool<ShaderTag, ShaderHotData, ShaderColdData> mShaderPool;
        ResourcePool<GPUBufferTag, GPUBufferHotData, GPUBufferColdData> mGPUBufferPool;
    };

    template<> void ResourceManager::AddRef<ShaderTag>(ShaderHandle handle);
    template<> void ResourceManager::DestroyRef<ShaderTag>(ShaderHandle handle);

    template<> void ResourceManager::AddRef<GPUBufferTag>(GPUBufferHandle handle);
    template<> void ResourceManager::DestroyRef<GPUBufferTag>(GPUBufferHandle handle);


}
