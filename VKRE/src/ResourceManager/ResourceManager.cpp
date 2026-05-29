#include <ResourceManager/ResourceManager.h>
#include <ResourceManager/ResourceRefs.h>
#include <ResourceManager/Resources.h>

#include <print>
#include <algorithm>

namespace VKRE {

    ResourceManager::ResourceManager() {
        mShaderPool.Init(INITIAL_SHADER_POOL_CAP);
        mGPUBufferPool.Init(INITIAL_GPU_BUFFER_POOL_CAP);
    }

    ResourceManager::~ResourceManager() {
        uint32_t leakedShaders = mShaderPool.GetLiveCount();
        if (leakedShaders > 0) std::println("ResourceManager: {} shader(s) were not explicitly destroyed before shutdown", leakedShaders);

        uint32_t leakedGPUBuffers = mGPUBufferPool.GetLiveCount();
        if (leakedGPUBuffers > 0) std::println("ResourceManager: {} GPUBuffer(s) were not explicitly destroyed before shutdown", leakedGPUBuffers);
    }

    template<>
    void ResourceManager::AddRef<ShaderTag>(ShaderHandle handle) {
        if (!mShaderPool.IsValid(handle)) {
            std::println("ResourceManager::AddRef<ShaderTag> handle is invalid or already destroyed");
            return;
        }

        mShaderPool.AddRef(handle);
    }

    template<>
    void ResourceManager::DestroyRef<ShaderTag>(ShaderHandle handle) {
        if (!mShaderPool.IsValid(handle)) {
            std::println("ResourceManager::DestroyRef<ShaderTag> handle is invalid or already destroyed");
            return;
        }

        if (!mShaderPool.RemoveRef(handle)) return;

        ShaderColdData* cold = mShaderPool.GetCold(handle);
        cold->ByteCode.clear();
        cold->ByteCode.shrink_to_fit();

        mShaderPool.Free(handle);
    }

    template<>
    void ResourceManager::AddRef<GPUBufferTag>(GPUBufferHandle handle) {
        if (!mGPUBufferPool.IsValid(handle)) {
            std::println("ResourceManager::AddRef<GPUBufferTag> handle is invalid or already destroyed");
            return;
        }

        mGPUBufferPool.AddRef(handle);
    }

    template<>
    void ResourceManager::DestroyRef<GPUBufferTag>(GPUBufferHandle handle) {
        if (!mGPUBufferPool.IsValid(handle)) {
            std::println("ResourceManager::Destroy<GPUBufferTag> handle is invalid or already destroyed");
            return;
        }

        if (!mGPUBufferPool.RemoveRef(handle)) return;
        mGPUBufferPool.Free(handle);
    }

    ResourceRef<ShaderTag> ResourceManager::LoadShader(ShaderDesc& desc) {
        ShaderHandle handle = mShaderPool.FindIf([&](const ShaderHotData& hot, const ShaderColdData& cold) {
            return hot.Stage == desc.Stage && std::strcmp(cold.Path, desc.Path.c_str()) == 0;
        });

        if (handle.IsValid()) {
            return ResourceRef<ShaderTag>(handle, this);
        }

        handle = mShaderPool.Allocate();

        ShaderHotData* hot = mShaderPool.GetHot(handle);
        ShaderColdData* cold = mShaderPool.GetCold(handle);

        if (!hot || !cold) {
            std::println("Shader allocation failed ({}): Shader pointers are invalid", desc.DebugName);
            assert(false);
        }

        cold->ByteCode = std::move(desc.ByteCode);
        hot->Stage = desc.Stage;

        size_t len = 0;
        len = std::min(desc.Entrypoint.length(), sizeof(cold->Entrypoint) - 1);
        std::copy_n(desc.Entrypoint.begin(), len, cold->Entrypoint);
        cold->Entrypoint[len] = '\0';

        len = std::min(desc.DebugName.length(), sizeof(cold->DebugName) - 1);
        std::copy_n(desc.DebugName.begin(), len, cold->DebugName);
        cold->DebugName[len] = '\0';

        len = std::min(desc.Path.length(), sizeof(cold->Path) - 1);
        std::copy_n(desc.Path.begin(), len, cold->Path);
        cold->Path[len] = '\0';

        cold->IsDirty = false;

        return ResourceRef<ShaderTag>(handle, this);
    }

    void ResourceManager::MarkShaderDirty(ShaderHandle handle) {
        if (!mShaderPool.IsValid(handle)) {
            std::println("ResourceManager::MakeShaderDirty handle is invalid");
            return;
        }

        ShaderColdData* cold = mShaderPool.GetCold(handle);
        cold->IsDirty = true;
    }

    ResourceRef<GPUBufferTag> ResourceManager::CreateGPUBuffer(GPUBufferDesc& desc) {
        GPUBufferHandle handle = mGPUBufferPool.Allocate();

        GPUBufferHotData* hot = mGPUBufferPool.GetHot(handle);
        GPUBufferColdData* cold = mGPUBufferPool.GetCold(handle);

        if (!hot || !cold) {
            std::println("GPUBuffer allocation failed ({}): GPUBuffer pointers are invalid", desc.DebugName);
            assert(false);
        }

        cold->Usage = desc.Usage;
        hot->Size = desc.Size;
        hot->HostVisible = desc.HostVisible;

        size_t len = std::min(desc.DebugName.length(), sizeof(cold->DebugName) - 1);
        std::copy_n(desc.DebugName.begin(), len, cold->DebugName);
        cold->DebugName[len] = '\0';

        return ResourceRef<GPUBufferTag>(handle, this);
    }

}
