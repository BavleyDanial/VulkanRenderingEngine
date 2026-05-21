#include "ResourceManager/Resources.h"
#include <ResourceManager/ResourceManager.h>

#include <print>
#include <algorithm>

namespace VKRE {

    ResourceManager::ResourceManager() {
        mShaderPool.Init(INITIAL_SHADER_POOL_CAP);
    }

    ResourceManager::~ResourceManager() {
        uint32_t leakedShaders = mShaderPool.GetLiveCount();
        if (leakedShaders > 0) std::println("ResourceManager: {} shader(s) were not explicitly destroyed before shutdown", leakedShaders);
    }

    ShaderHandle ResourceManager::LoadShader(ShaderDesc&& desc) {
        ShaderHandle handle = mShaderPool.FindIf([&](const ShaderHotData& hot, const ShaderColdData& cold) {
            return hot.stage == desc.stage && std::strcmp(cold.path, desc.path.c_str()) == 0;
        });

        if (handle.IsValid()) {
            mShaderPool.AddRef(handle);
            return handle;
        }

        handle = mShaderPool.Allocate();

        ShaderHotData* hot = mShaderPool.GetHot(handle);
        ShaderColdData* cold = mShaderPool.GetCold(handle);

        std::string error = "Shader allocation failed (" + desc.debugName + "): Shader pointers are invalid";
        assert(hot && cold && error.c_str());

        cold->byteCode = std::move(desc.byteCode);
        hot->stage = desc.stage;

        size_t len = 0;
        len = std::min(desc.entrypoint.length(), sizeof(hot->entrypoint) - 1);
        std::copy_n(desc.entrypoint.begin(), len, hot->entrypoint);
        hot->entrypoint[len] = '\0';

        len = std::min(desc.debugName.length(), sizeof(cold->debugName) - 1);
        std::copy_n(desc.debugName.begin(), len, cold->debugName);
        cold->debugName[len] = '\0';

        len = std::min(desc.path.length(), sizeof(cold->path) - 1);
        std::copy_n(desc.path.begin(), len, cold->path);
        cold->path[len] = '\0';

        cold->isDirty = false;

        return handle;
    }

    void ResourceManager::DestroyShaderRef(ShaderHandle handle) {
        if (!mShaderPool.IsValid(handle)) {
            std::println("ResourceManager::DestroyShader handle is invalid or already destroyed");
            return;
        }

        if (!mShaderPool.RemoveRef(handle)) return;

        ShaderColdData* cold = mShaderPool.GetCold(handle);
        cold->byteCode.clear();
        cold->byteCode.shrink_to_fit();

        mShaderPool.Free(handle);
    }

    void ResourceManager::MarkShaderDirty(ShaderHandle handle) {
        if (!mShaderPool.IsValid(handle)) {
            std::println("ResourceManager::MakeShaderDirty handle is invalid");
            return;
        }

        ShaderColdData* cold = mShaderPool.GetCold(handle);
        cold->isDirty = true;
    }

}
