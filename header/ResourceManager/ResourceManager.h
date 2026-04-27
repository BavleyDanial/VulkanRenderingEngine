#pragma once

#include "ResourceHandles.h"
#include "ResourcePool.h"
#include "Resources.h"

namespace VKRE {

    class ResourceManager {
        ResourceManager();
        ~ResourceManager();

        ShaderHandle CreateShader(ShaderDesc&& desc);
        void DestroyShader(ShaderHandle handle);

        ShaderHotData* GetShaderHot(ShaderHandle handle) { return mShaderPool.GetHot(handle); };
        const ShaderHotData* GetShaderHot(ShaderHandle handle) const { return mShaderPool.GetHot(handle); };
        ShaderColdData* GetShaderCold(ShaderHandle handle) { return mShaderPool.GetCold(handle); };
        const ShaderColdData* GetShaderCold(ShaderHandle handle) const { return mShaderPool.GetCold(handle); };

        void MarkShaderDirty(ShaderHandle handle);

        // NOTE: Maybe turn both of these into a template for all other types?
        bool IsShaderValid(ShaderHandle handle) const { return mShaderPool.IsValid(handle); }
        uint32_t GetLiveShaderCount() const;

    private:
        static constexpr uint32_t INITIAL_SHADER_POOL_CAP = 64;

        ResourcePool<ShaderTag, ShaderHotData, ShaderColdData> mShaderPool;
    };

}
