#pragma once

#include <ResourceManager/ResourceManager.h>

#include <AssetsManagers/MeshImporter.h>
#include <AssetsManagers/ShaderImporter.h>
#include <AssetsManagers/TextureImporter.h>

#include <Assets/Mesh.h>
#include <Assets/Shader.h>
#include <Assets/Texture.h>
//#include <Assets/Materials.h>

#include <filesystem>
#include <unordered_map>

namespace VKRE {

    class AssetManager {
    public:
        AssetManager(ResourceManager& manager);
        ~AssetManager();

        static const MeshAsset* LoadMesh(const std::filesystem::path& path, const MeshImportOptions& options = {});
        static const ShaderAsset* LoadShader(const std::filesystem::path& path, const ShaderImportOptions& options = {});
        static const Texture2DAsset* LoadTexture2D(const std::filesystem::path& path, const TextureImportOptions& options = {});

    private:
        const MeshAsset* LoadMeshImpl(const std::filesystem::path& path, const MeshImportOptions& options);
        const ShaderAsset* LoadShaderImpl(const std::filesystem::path& path, const ShaderImportOptions& options);
        const Texture2DAsset* LoadTexture2DImpl(const std::filesystem::path& path, const TextureImportOptions& options);

    private:
        ResourceManager& mResourceManager;

        // TODO: Use something other than unordered_map or a custom data struct to store these
        std::unordered_map<std::string, MeshAsset> mMeshAssets;
        std::unordered_map<std::string, ShaderAsset> mShaderAssets;
        std::unordered_map<std::string, Texture2DAsset> mTexture2DAssets;

        inline static AssetManager* sInstance = nullptr;
    };

}
