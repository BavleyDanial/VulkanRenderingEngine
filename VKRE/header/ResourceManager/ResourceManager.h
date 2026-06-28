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

        ResourceRef<ShaderTag> CreateShader(ShaderDesc& desc);

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

        ResourceRef<Texture2DTag> CreateTexture2D(TextureDesc& desc);

        TextureHotData* GetTexture2DHot(Texture2DHandle handle) { return mTexture2DPool.GetHot(handle); };
        const TextureHotData* GetTexture2DHot(Texture2DHandle handle) const { return mTexture2DPool.GetHot(handle); };
        TextureColdData* GetTexture2DCold(Texture2DHandle handle) { return mTexture2DPool.GetCold(handle); };
        const TextureColdData* GetTexture2DCold(Texture2DHandle handle) const { return mTexture2DPool.GetCold(handle); };

        bool IsTexture2DValid(Texture2DHandle handle) const { return mTexture2DPool.IsValid(handle); }
        uint32_t GetLiveTexture2DCount() const { return mTexture2DPool.GetLiveCount(); }

        ResourceRef<TextureCubeTag> CreateTextureCube(TextureDesc& desc);

        TextureHotData* GetTextureCubeHot(TextureCubeHandle handle) { return mTextureCubePool.GetHot(handle); };
        const TextureHotData* GetTextureCubeHot(TextureCubeHandle handle) const { return mTextureCubePool.GetHot(handle); };
        TextureColdData* GetTextureCubeCold(TextureCubeHandle handle) { return mTextureCubePool.GetCold(handle); };
        const TextureColdData* GetTextureCubeCold(TextureCubeHandle handle) const { return mTextureCubePool.GetCold(handle); };

        bool IsTextureCubeValid(TextureCubeHandle handle) const { return mTextureCubePool.IsValid(handle); }
        uint32_t GetLiveTextureCubeCount() const { return mTextureCubePool.GetLiveCount(); }

    private:
        static constexpr uint32_t INITIAL_SHADER_POOL_CAP = 64;
        static constexpr uint32_t INITIAL_GPU_BUFFER_POOL_CAP = 64;
        static constexpr uint32_t INITIAL_TEXTURE_2D_POOL_CAP = 64;
        static constexpr uint32_t INITIAL_TEXTURE_CUBE_POOL_CAP = 64;

        ResourcePool<ShaderTag, ShaderHotData, ShaderColdData> mShaderPool;
        ResourcePool<GPUBufferTag, GPUBufferHotData, GPUBufferColdData> mGPUBufferPool;
        ResourcePool<Texture2DTag, TextureHotData, TextureColdData> mTexture2DPool;
        ResourcePool<TextureCubeTag, TextureHotData, TextureColdData> mTextureCubePool;
    };

    template<> void ResourceManager::AddRef<ShaderTag>(ShaderHandle handle);
    template<> void ResourceManager::DestroyRef<ShaderTag>(ShaderHandle handle);

    template<> void ResourceManager::AddRef<GPUBufferTag>(GPUBufferHandle handle);
    template<> void ResourceManager::DestroyRef<GPUBufferTag>(GPUBufferHandle handle);

    template<> void ResourceManager::AddRef<Texture2DTag>(Texture2DHandle handle);
    template<> void ResourceManager::DestroyRef<Texture2DTag>(Texture2DHandle handle);

    template<> void ResourceManager::AddRef<TextureCubeTag>(TextureCubeHandle handle);
    template<> void ResourceManager::DestroyRef<TextureCubeTag>(TextureCubeHandle handle);

}
