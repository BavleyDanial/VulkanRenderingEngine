#include "ResourceManager/Resources.h"
#include <AssetsManagers/AssetManager.h>

#include <Renderer/Renderer.h>

namespace VKRE {

    AssetManager::AssetManager(ResourceManager& manager)
        :mResourceManager(manager) {
        assert(!sInstance && "AssetManager has been initiated already");
        sInstance = this;
    }

    AssetManager::~AssetManager() {
        mMeshAssets.clear();
        mShaderAssets.clear();
    }

    const MeshAsset* AssetManager::LoadMesh(const std::filesystem::path& path, const MeshImportOptions& options) {
        assert(sInstance && "AssetManager hasn't been initiated");
        return sInstance->LoadMeshImpl(path, options);
    }

    const MeshAsset* AssetManager::LoadMeshImpl(const std::filesystem::path& path, const MeshImportOptions& options) {
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

        std::filesystem::path meshDir = path.parent_path();
        for (const std::string& texPath : importedMesh.TexturePaths) {
            std::filesystem::path fullTexPath = meshDir / texPath;
            fullTexPath = std::filesystem::path(fullTexPath).lexically_normal();
            const Texture2DAsset* texAsset = LoadTexture2D(fullTexPath, { .GenerateMipMaps = true });
            if (texAsset) {
                meshAsset.Textures.push_back(texAsset->Texture);
                meshAsset.TexturesIndices.push_back(texAsset->BindlessIndex);
            } else {
                meshAsset.Textures.push_back(ResourceRef<Texture2DTag>());
                meshAsset.TexturesIndices.push_back(-1);
            }
        }

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

    const ShaderAsset* AssetManager::LoadShader(const std::filesystem::path& path, const ShaderImportOptions& options) {
        assert(sInstance && "AssetManager hasn't been initiated");
        return sInstance->LoadShaderImpl(path, options);
    }

    const ShaderAsset* AssetManager::LoadShaderImpl(const std::filesystem::path& path, const ShaderImportOptions& options) {
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

    const Texture2DAsset* AssetManager::LoadTexture2D(const std::filesystem::path& path, const TextureImportOptions& options) {
        assert(sInstance && "AssetManager hasn't been initiated");
        return sInstance->LoadTexture2DImpl(path, options);
    }

    const Texture2DAsset* AssetManager::LoadTexture2DImpl(const std::filesystem::path& path, const TextureImportOptions& options) {
        std::string pathStr = path.string();

        auto it = mTexture2DAssets.find(pathStr);
        if (it != mTexture2DAssets.end()) return &it->second;

        std::optional<Texture2DImportResults> importedTextureResults = TextureImporter::LoadTexture2DFromFile(path);
        if (!importedTextureResults.has_value())
            return nullptr;

        Texture2DImportResults importedTexture = std::move(importedTextureResults.value());

        Texture2DAsset textureAsset;
        textureAsset.Name = importedTexture.Name;
        textureAsset.Path = pathStr;
        textureAsset.Width = importedTexture.Width;
        textureAsset.Height = importedTexture.Height;
        textureAsset.Format = importedTexture.Format;
        textureAsset.MipLevels = options.GenerateMipMaps ? static_cast<uint32_t>(glm::floor(glm::log2((float)glm::max(importedTexture.Width, importedTexture.Height)))) + 1 : 1;

        TextureDesc desc{};
        desc.Format = importedTexture.Format;
        desc.Usage = TextureUsage::Sampled | TextureUsage::TransferSrc | TextureUsage::TransferDst;
        desc.DebugName = importedTexture.Name;
        desc.Dimensions = { importedTexture.Width, importedTexture.Height, 1 };
        desc.MipLevels = textureAsset.MipLevels;

        ResourceRef<Texture2DTag> texture = mResourceManager.CreateTexture2D(desc);
        textureAsset.Texture = texture;
        textureAsset.BindlessIndex = Renderer::UploadTexture2D(texture.Get(), importedTexture.Data);

        auto result = mTexture2DAssets.emplace(pathStr, std::move(textureAsset));
        return &result.first->second;
    }

    const TextureCubeAsset* AssetManager::LoadTextureCube(const std::filesystem::path& path, const TextureImportOptions& options) {
        assert(sInstance && "AssetManager hasn't been initiated");
        return sInstance->LoadTextureCubeImpl(path, options);
    }

    const TextureCubeAsset* AssetManager::LoadTextureCube(const std::array<std::filesystem::path, 6>& paths, const TextureImportOptions& options) {
        assert(sInstance && "AssetManager hasn't been initiated");
        return sInstance->LoadTextureCubeImpl(paths, options);
    }

    const TextureCubeAsset* AssetManager::LoadTextureCubeImpl(const std::filesystem::path& path, const TextureImportOptions& options) {
        std::string pathStr = path.string();

        auto it = mTextureCubeAssets.find(pathStr);
        if (it != mTextureCubeAssets.end()) return &it->second;

        std::optional<TextureCubeImportResults> importedTextureResults = TextureImporter::LoadTextureCubeFromHorizontalCross(path, options);
        if (!importedTextureResults.has_value())
            return nullptr;

        return FillTextureCubeData(importedTextureResults.value(), pathStr, options);
    }

    const TextureCubeAsset* AssetManager::LoadTextureCubeImpl(const std::array<std::filesystem::path, 6>& paths, const TextureImportOptions& options) {
        std::string pathStr = paths[0].parent_path().stem().string();

        auto it = mTextureCubeAssets.find(pathStr);
        if (it != mTextureCubeAssets.end()) return &it->second;

        std::optional<TextureCubeImportResults> importedTextureResults = TextureImporter::LoadTextureCubeFromFiles(paths, options);
        if (!importedTextureResults.has_value())
            return nullptr;

        return FillTextureCubeData(importedTextureResults.value(), pathStr, options);
    }

    const TextureCubeAsset* AssetManager::FillTextureCubeData(const TextureCubeImportResults& importedTexture,
                                                        const std::string& pathStr, const TextureImportOptions& options) {
        TextureCubeAsset textureAsset;
        textureAsset.Name = importedTexture.Name;
        textureAsset.Path = pathStr;
        textureAsset.Width = importedTexture.Width;
        textureAsset.Height = importedTexture.Height;
        textureAsset.Format = importedTexture.Format;
        textureAsset.MipLevels = options.GenerateMipMaps ? static_cast<uint32_t>(glm::floor(glm::log2((float)glm::max(importedTexture.Width, importedTexture.Height)))) + 1 : 1;

        TextureDesc desc{};
        desc.Format = importedTexture.Format;
        desc.Usage = TextureUsage::Sampled | TextureUsage::TransferSrc | TextureUsage::TransferDst;
        desc.DebugName = importedTexture.Name;
        desc.Dimensions = { importedTexture.Width, importedTexture.Height, 1 };
        desc.MipLevels = textureAsset.MipLevels;

        ResourceRef<TextureCubeTag> texture = mResourceManager.CreateTextureCube(desc);
        textureAsset.Texture = texture;
        textureAsset.BindlessIndex = Renderer::UploadTextureCube(texture.Get(), importedTexture.Data);

        auto result = mTextureCubeAssets.emplace(pathStr, std::move(textureAsset));
        return &result.first->second;
    }

}
