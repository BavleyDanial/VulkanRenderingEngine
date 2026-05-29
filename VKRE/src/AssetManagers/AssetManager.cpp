#include <AssetsManagers/AssetManager.h>

#include <AssetsManagers/MeshImporter.h>
#include <AssetsManagers/ShaderImporter.h>

#include <Renderer/Renderer.h>

namespace VKRE {

    AssetManager::AssetManager(ResourceManager& manager)
        :mResourceManager(manager) {
        assert(!sInstance && "AssetManager has been initiated already");
        sInstance = this;
    }

    AssetManager::~AssetManager() {
        mMeshAssets.clear();
    }

    const MeshAsset* AssetManager::LoadMesh(const std::filesystem::path& path) {
        assert(sInstance && "AssetManager hasn't been initiated");
        return sInstance->LoadMeshImpl(path);
    }

    const MeshAsset* AssetManager::LoadMeshImpl(const std::filesystem::path& path) {
        std::string pathStr = path.string();

        auto it = mMeshAssets.find(pathStr);
        if (it != mMeshAssets.end()) return &it->second;

        std::optional<MeshImportResults> importedMeshResults = MeshImporter::LoadFromFile(path);
        if (!importedMeshResults.has_value())
            return nullptr;

        MeshImportResults importedMesh = std::move(importedMeshResults.value());

        MeshAsset meshAsset;
        meshAsset.Name = importedMesh.Name;
        meshAsset.Path = pathStr;
        meshAsset.SubMeshes = std::move(importedMesh.SubMeshes);
        meshAsset.Nodes = std::move(importedMesh.Nodes);
        meshAsset.NodeSubMeshIndices = std::move(importedMesh.NodeSubMeshIndices);
        meshAsset.NodeNames = std::move(importedMesh.NodeNames);

        GPUBufferDesc vbDesc{};
        vbDesc.DebugName = importedMesh.Name + "_VBO";
        vbDesc.Size = static_cast<uint64_t>(importedMesh.Vertices.size()) * sizeof(Vertex);
        vbDesc.Usage = GPUBufferUsage::Vertex | GPUBufferUsage::TransferDst;
        vbDesc.HostVisible = false;

        GPUBufferDesc ibDesc{};
        ibDesc.DebugName = importedMesh.Name + "_IBO";
        ibDesc.Size = static_cast<uint64_t>(importedMesh.Indices.size()) * sizeof(uint32_t);
        ibDesc.Usage = GPUBufferUsage::Index | GPUBufferUsage::TransferDst;
        ibDesc.HostVisible = false;

        meshAsset.VertexBuffer = mResourceManager.CreateGPUBuffer(vbDesc);
        meshAsset.IndexBuffer = mResourceManager.CreateGPUBuffer(ibDesc);

        Renderer::UploadMesh(meshAsset.VertexBuffer.Get(), meshAsset.IndexBuffer.Get(), importedMesh.Vertices, importedMesh.Indices);

        auto result = mMeshAssets.emplace(pathStr, std::move(meshAsset));
        return &result.first->second;
    }

    const ShaderAsset* AssetManager::LoadShader(const std::filesystem::path& path) {
        assert(sInstance && "AssetManager hasn't been initiated");
        return sInstance->LoadShaderImpl(path);
    }

    const ShaderAsset* AssetManager::LoadShaderImpl(const std::filesystem::path& path) {
        std::string pathStr = path.string();

        auto it = mShaderAssets.find(pathStr);
        if (it != mShaderAssets.end()) return &it->second;

        std::optional<ShaderImportResults> importedShaderResults = ShaderImporter::LoadFromFile(mResourceManager, path);
        if (!importedShaderResults.has_value())
            return nullptr;

        ShaderImportResults importedShader = std::move(importedShaderResults.value());

        ShaderAsset shaderAsset;
        shaderAsset.Name = importedShader.Name;
        shaderAsset.Path = pathStr;
        shaderAsset.VertexShader = std::move(importedShader.VertexShader);
        shaderAsset.FragmentShader = std::move(importedShader.FragmentShader);
        shaderAsset.ComputeShader = std::move(importedShader.ComputeShader);

        auto result = mShaderAssets.emplace(pathStr, std::move(shaderAsset));
        return &result.first->second;
    }


}
