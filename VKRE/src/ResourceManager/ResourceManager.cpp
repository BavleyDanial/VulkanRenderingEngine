#include <ResourceManager/ResourceManager.h>
#include <ResourceManager/ResourceRefs.h>
#include <ResourceManager/Resources.h>

#include <print>
#include <algorithm>

namespace VKRE {

    ResourceManager::ResourceManager() {
        mShaderPool.Init(INITIAL_SHADER_POOL_CAP);
        mGPUBufferPool.Init(INITIAL_GPU_BUFFER_POOL_CAP);
        mMeshPool.Init(INITIAL_MESH_POOL_CAP);
    }

    ResourceManager::~ResourceManager() {
        uint32_t leakedShaders = mShaderPool.GetLiveCount();
        if (leakedShaders > 0) std::println("ResourceManager: {} shader(s) were not explicitly destroyed before shutdown", leakedShaders);

        uint32_t leakedGPUBuffers = mGPUBufferPool.GetLiveCount();
        if (leakedGPUBuffers > 0) std::println("ResourceManager: {} GPUBuffer(s) were not explicitly destroyed before shutdown", leakedGPUBuffers);

        uint32_t leakedMeshes = mMeshPool.GetLiveCount();
        if (leakedMeshes > 0) std::println("ResourceManager: {} Mesh(es) were not explicitly destroyed before shutdown", leakedMeshes);
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

    template<>
    void ResourceManager::AddRef<MeshTag>(MeshHandle handle) {
        if (!mMeshPool.IsValid(handle)) {
            std::println("ResourceManager::AddRef<GPUBufferTag> handle is invalid or already destroyed");
            return;
        }

        mMeshPool.AddRef(handle);
    }

    template<>
    void ResourceManager::DestroyRef<MeshTag>(MeshHandle handle) {
        if (!mMeshPool.IsValid(handle)) {
            std::println("ResourceManager::Destroy<GPUBufferTag> handle is invalid or already destroyed");
            return;
        }

        if (!mMeshPool.RemoveRef(handle)) return;

        // Remove extra reference count to destroy the GPUBuffer
        MeshHotData* hot = GetMeshHot(handle);
        DestroyRef(hot->VertexBuffer);
        DestroyRef(hot->IndexBuffer);
        mMeshPool.Free(handle);
    }

    ResourceRef<ShaderTag> ResourceManager::LoadShader(ShaderDesc&& desc) {
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
        len = std::min(desc.Entrypoint.length(), sizeof(hot->Entrypoint) - 1);
        std::copy_n(desc.Entrypoint.begin(), len, hot->Entrypoint);
        hot->Entrypoint[len] = '\0';

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

    ResourceRef<GPUBufferTag> ResourceManager::LoadGPUBuffer(GPUBufferDesc&& desc) {
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

    ResourceRef<MeshTag> ResourceManager::LoadMesh(MeshDesc&& desc) {
        MeshHandle handle = mMeshPool.Allocate();

        MeshHotData* hot = mMeshPool.GetHot(handle);
        MeshColdData* cold = mMeshPool.GetCold(handle);

        if (!hot || !cold) {
            std::println("Mesh allocation failed ({}): Mesh pointers are invalid", desc.DebugName);
            assert(false);
        }

        ResourceRef<GPUBufferTag> vertexBuffer = LoadGPUBuffer({
            .DebugName = desc.DebugName + "_vtx",
            .Size = desc.Vertices.size() * sizeof(Vertex),
            .Usage = GPUBufferUsage::Vertex | GPUBufferUsage::TransferDst,
            .HostVisible = false,
        });

        ResourceRef<GPUBufferTag> indexBuffer = LoadGPUBuffer({
            .DebugName = desc.DebugName + "_idx",
            .Size = desc.Indices.size() * sizeof(uint32_t),
            .Usage = GPUBufferUsage::Index | GPUBufferUsage::TransferDst,
            .HostVisible = false,
        });

        // To prevent GPUBuffer reference count from going to 0 before we return... In DestroyRef<MeshTag> we do an extra DestroyRef to counter this added one
        mGPUBufferPool.AddRef(vertexBuffer.Get());
        mGPUBufferPool.AddRef(indexBuffer.Get());

        hot->VertexBuffer = vertexBuffer.Get();
        hot->IndexBuffer = indexBuffer.Get();

        hot->VerticesCount = static_cast<uint32_t>(desc.Vertices.size());
        hot->IndicesCount = static_cast<uint32_t>(desc.Indices.size());

        size_t len = std::min(desc.DebugName.length(), sizeof(cold->DebugName) - 1);
        std::copy_n(desc.DebugName.begin(), len, cold->DebugName);
        cold->DebugName[len] = '\0';

        return ResourceRef<MeshTag>(handle, this);
    }
}
