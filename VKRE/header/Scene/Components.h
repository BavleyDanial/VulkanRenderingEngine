#pragma once

#include <glm/glm.hpp>

#include <ResourceManager/ResourceHandles.h>

namespace VKRE {

    struct TransformComponent {
        glm::vec3 Position{0.0f};
        glm::vec3 Rotation{0.0f};
        glm::vec3 Scale {1.0f};

        glm::mat4 WorldMatrix{1.0f};
    };

    struct StaticMeshComponent {
        MeshHandle Mesh;
    };

}
