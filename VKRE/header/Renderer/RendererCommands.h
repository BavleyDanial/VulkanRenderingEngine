#pragma once

#include <ResourceManager/ResourceHandles.h>

#include <glm/glm.hpp>
#include <cstdint>

namespace VKRE {

    struct DrawPushConstants {
        glm::mat4 Transform = glm::mat4(1.0f);
        uint64_t VertexBufferAddress = 0;
        int32_t Texture2DIndex = -1;
        int32_t TextureCubeIndex = -1;
    };

    struct SkyboxPushConstants {
        glm::mat4 ViewProjection = glm::mat4(1.0f);
        int32_t TextureCubeIndex = -1;
    };

    struct MeshDrawCommand {
        GPUBufferHandle IndexBuffer;
        uint32_t IndexCount;
        uint32_t BaseIndex;
        uint32_t BaseVertex;
        uint64_t VertexBufferAddress;
        glm::mat4 Transform;
        glm::mat4 ViewProjection;
        int32_t TextureIndex = -1;
    };

    struct SkyboxDrawCommand {
        SkyboxPushConstants PushConstants;
    };

}
