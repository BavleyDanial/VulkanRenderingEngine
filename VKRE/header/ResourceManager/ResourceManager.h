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

        ResourceRef<ShaderTag> LoadShader(ShaderDesc&& desc);

        ShaderHotData* GetShaderHot(ShaderHandle handle) { return mShaderPool.GetHot(handle); };
        const ShaderHotData* GetShaderHot(ShaderHandle handle) const { return mShaderPool.GetHot(handle); };
        ShaderColdData* GetShaderCold(ShaderHandle handle) { return mShaderPool.GetCold(handle); };
        const ShaderColdData* GetShaderCold(ShaderHandle handle) const { return mShaderPool.GetCold(handle); };

        void MarkShaderDirty(ShaderHandle handle);

        bool IsShaderValid(ShaderHandle handle) const { return mShaderPool.IsValid(handle); }
        uint32_t GetLiveShaderCount() const { return mShaderPool.GetLiveCount(); }

        template<typename Tag>
        void AddRef(ResourceHandle<Tag> handle);
        template<typename Tag>
        void DestroyRef(ResourceHandle<Tag> handle);

    private:
        static constexpr uint32_t INITIAL_SHADER_POOL_CAP = 64;
        ResourcePool<ShaderTag, ShaderHotData, ShaderColdData> mShaderPool;
    };

    template<> void ResourceManager::AddRef<ShaderTag>(ShaderHandle handle);
    template<> void ResourceManager::DestroyRef<ShaderTag>(ShaderHandle handle);

}
