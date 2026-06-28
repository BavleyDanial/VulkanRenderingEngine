#pragma once

#include <Vulkan/VulkanRenderer.h>
#include <ResourceManager/ResourceManager.h>

#include <Renderer/RendererCommands.h>

#include <cstddef>
#include <glm/glm.hpp>

namespace VKRE {

    class Renderer {
    public:
        static void SetRenderer(VulkanRenderer* renderer) {
            sRenderer = renderer;
        }

        static glm::vec2 GetViewportDimensions() {
            return sRenderer->GetViewportDimensions();
        }

        static void UploadMesh(GPUBufferHandle VertexBuffer, GPUBufferHandle IndexBuffer, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
            sRenderer->UploadMesh(VertexBuffer, IndexBuffer, vertices, indices);
        }

        static int32_t UploadTexture2D(Texture2DHandle handle, const std::vector<std::byte>& pixels) {
            return sRenderer->UploadTexture2D(handle, pixels);
        }

        static int32_t UploadTextureCube(TextureCubeHandle handle, const std::array<std::vector<std::byte>, 6>& faces) {
            return sRenderer->UploadTextureCube(handle, faces);
        }

        static void UploadSceneData(const SceneUBO& sceneData) {
            sRenderer->UploadSceneData(sceneData);
        }

        static void SubmitMeshDraw(DrawPassHandle handle, const MeshDrawCommand& cmd) {
            sRenderer->SubmitMeshDraw(handle, cmd);
        }

        static void SubmitSkyboxDraw(DrawPassHandle handle, const SkyboxDrawCommand& cmd) {
            sRenderer->SubmitSkyboxDraw(handle, cmd);
        }

        static uint64_t GetBufferDeviceAddress(GPUBufferHandle handle) {
            return sRenderer->GetBufferDeviceAddress(handle);
        }

        static DrawPassHandle AddDrawPass(const DrawPassDesc& desc) {
            return sRenderer->AddDrawPass(desc);
        }

        static ComputePassHandle AddComputePass(const ComputePassDesc& desc) {
            return sRenderer->AddComputePass(desc);
        }

        static void SetComputePassData(ComputePassHandle handle, void* data, uint32_t size) {
            sRenderer->SetComputePassData(handle, data, size);
        }

        static void ActivateDrawPass(DrawPassHandle handle) {
            sRenderer->ActivateDrawPass(handle);
        }

        static void DeActivateDrawPass(DrawPassHandle handle) {
            sRenderer->DeActivateDrawPass(handle);
        }

        static void ActivateComputePass(ComputePassHandle handle) {
            sRenderer->ActivateComputePass(handle);
        }

        static void DeActivateComputePass(ComputePassHandle handle) {
            sRenderer->DeActivateComputePass(handle);
        }

    private:
        inline static VulkanRenderer* sRenderer = nullptr;
        inline static ResourceManager* sResourceManager = nullptr;
    };

}
