#pragma once

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <Assets/Mesh.h>
#include <ResourceManager/Resources.h>

#include <filesystem>
#include <optional>
#include <vector>

namespace VKRE {

    struct MeshImportOptions {
        // TODO: Add import options of optimization, flipping uv, etc
    };

    struct MeshImportResults {
        std::string Name;

        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
        std::vector<SubMesh> SubMeshes;

        std::vector<MeshNode> Nodes;
        std::vector<std::string> NodeNames;
        std::vector<uint32_t> NodeSubMeshIndices;

        std::vector<std::string> TexturePaths;
    };

    class MeshImporter {
    public:
        static std::optional<MeshImportResults> LoadFromFile(const std::filesystem::path& path);

    private:
        static MeshImportResults& FlattenNodeHierarchy(const aiNode* node, int32_t ParentIndex, MeshImportResults& results);

    private:
        static glm::mat4 ToGLM(const aiMatrix4x4& mat);
        static glm::mat3 ToGLM(const aiMatrix3x3& mat);
        static glm::vec4 ToGLM(const aiColor4D& vec);
        static glm::vec3 ToGLM(const aiVector3D& vec);
        static glm::vec2 ToGLM(const aiVector2D& vec);

    };

}
