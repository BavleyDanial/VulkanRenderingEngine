#include <Renderer/SceneRenderer.h>

#include <Scene/Components.h>
#include <ResourceManager/Resources.h>

#include <Renderer/Renderer.h>

#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>

#include <print>

namespace VKRE {

    SceneRenderer::SceneRenderer() {
        mDrawPass = Renderer::AddDrawPass({
            .shaderPath = "res/shaders/mesh.glsl",
            .debugName = "Scene Draw Pass",
            .pushConstantRanges = { { VK_SHADER_STAGE_VERTEX_BIT, 0, 144 } },
            .colorAttachmentFormats = { VK_FORMAT_R16G16B16A16_SFLOAT },
            .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT
        });
    }

    void SceneRenderer::Render() {
        if (!mScene) {
            std::println("Couldn't find a scene to render");
            return;
        }

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        if (mCamera.IsValid()) {
            const TransformComponent& transform = mCamera.Get<TransformComponent>();
            const CameraComponent& camComponent = mCamera.Get<CameraComponent>();

            glm::vec3 position = transform.Position;
            glm::quat rotation = glm::quat(glm::radians(transform.Rotation));
            glm::vec3 forward = rotation * glm::vec3(0, 0, 1);

            view = glm::lookAt(position, position + forward, glm::vec3(0, 1, 0));
            projection = glm::perspective(glm::radians(camComponent.FOV), 1280.0f/720.0f, camComponent.Near, camComponent.Far);
            projection[0][0] *= -1; // flip the x axis
        }

        glm::mat4 vp = projection * view;

        mScene->GetFlecsWorld().each([&](const TransformComponent& transform, const StaticMeshComponent& staticMesh) {
            if (!staticMesh.Asset)
                return;

            uint64_t vbAddress = Renderer::GetBufferDeviceAddress(staticMesh.Asset->VertexBuffer.Get());
            if (vbAddress == 0) return;

            for (const auto& node : staticMesh.Asset->Nodes) {
                for (uint32_t i = 0; i < node.SubMeshCount; i++) {
                    uint32_t idx = staticMesh.Asset->NodeSubMeshIndices[i + node.SubMeshOffset];
                    const SubMesh& mesh = staticMesh.Asset->SubMeshes[idx];

                    MeshDrawCommand cmd{};
                    cmd.VertexBufferAddress = vbAddress;
                    cmd.IndexBuffer = staticMesh.Asset->IndexBuffer.Get();
                    cmd.IndexCount = mesh.IndexCount;
                    cmd.BaseIndex = mesh.BaseIndex;
                    cmd.BaseVertex = mesh.BaseVertex;
                    glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), transform.Position)
                    * glm::mat4_cast(glm::quat(glm::radians(transform.Rotation)))
                    * glm::scale(glm::mat4(1.0f), transform.Scale);

                    cmd.Transform = worldMatrix * node.LocalTransform;
                    cmd.ViewProjection = vp;

                    Renderer::SubmitMeshDraw(mDrawPass, cmd);
                }
            }
        });
    }

}
