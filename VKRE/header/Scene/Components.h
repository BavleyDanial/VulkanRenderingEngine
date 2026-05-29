#pragma once

#include <glm/glm.hpp>

#include <Assets/Mesh.h>

namespace VKRE {

    struct TransformComponent {
        glm::vec3 Position{0.0f};
        glm::vec3 Rotation{0.0f};
        glm::vec3 Scale {1.0f};

        glm::mat4 WorldMatrix{1.0f};
    };

    struct StaticMeshComponent {
        const MeshAsset* Asset;
    };

    struct CameraComponent {
        float FOV = 60.0f;
        float Near = 0.1f;
        float Far = 1000.0f;
    };

}
