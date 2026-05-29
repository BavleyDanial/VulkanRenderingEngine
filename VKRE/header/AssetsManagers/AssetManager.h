#pragma once

#include <ResourceManager/ResourceManager.h>

#include <Assets/Mesh.h>
//#include <Assets/Texture.h>
//#include <Assets/Materials.h>

#include <filesystem>
#include <unordered_map>

namespace VKRE {

    class AssetManager {
    public:
        AssetManager(ResourceManager& manager);
        ~AssetManager();

        static const MeshAsset* LoadMesh(const std::filesystem::path& path);

    private:
        const MeshAsset* LoadMeshImpl(const std::filesystem::path& path);

    private:
        ResourceManager& mResourceManager;
        std::unordered_map<std::string, MeshAsset> mMeshAssets;

        inline static AssetManager* sInstance = nullptr;
    };

}
