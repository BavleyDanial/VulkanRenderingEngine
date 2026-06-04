#pragma once

#include <ResourceManager/ResourceRefs.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

namespace VKRE {

    struct SubMesh {
        uint32_t BaseIndex;
        uint32_t IndexCount;
        int32_t BaseVertex;
        uint32_t VertexCount;
        int32_t TextureIndex = -1; // no texture bound by default
    };

    struct MeshNode {
        glm::mat4 LocalTransform;
        int32_t ParentIndex = -1;
        uint32_t SubMeshOffset;
        uint32_t SubMeshCount;
    };

    struct MeshAsset {
        std::string Name;
        std::string Path;

        std::vector<SubMesh> SubMeshes;
        ResourceRef<GPUBufferTag> VertexBuffer; 
        ResourceRef<GPUBufferTag> IndexBuffer;

        std::vector<MeshNode> Nodes;
        std::vector<uint32_t> NodeSubMeshIndices;
        std::vector<std::string> NodeNames;

        std::vector<ResourceRef<Texture2DTag>> Textures;
        std::vector<int32_t> TexturesIndices;
    };

}
