#pragma once

#include "Vulkan/VulkanRenderer.h"
#include "ResourceManager/ResourceManager.h"

#include <functional>
#include <vulkan/vulkan.h>

namespace VKRE {

    // NOTE: THIS IS TEMPORARY
    class Renderer {
    public:
        static void SetRenderer(VulkanRenderer* renderer) {
            sRenderer = renderer;
        }

        // NOTE: THIS IS EXTRA SUPER TEMPORARY
        static void SetResourceManager(ResourceManager* manager) {
            sResourceManager = manager;
        }

        // NOTE: THIS IS EXTRA SUPER TEMPORARY
        static void Submit(std::function<void(VkCommandBuffer)>&& fn) {
            return sRenderer->ImmediateSubmit(std::move(fn));
        }

        static void UploadMesh(ResourceRef<MeshTag> mMesh, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
            return sRenderer->UploadMesh(mMesh, vertices, indices);
        }
        // NOTE: THIS IS EXTRA SUPER TEMPORARY
        static ResourceRef<MeshTag> LoadMesh(MeshDesc&& desc) {
            return sResourceManager->LoadMesh(std::move(desc));
        }

        // NOTE: THIS IS EXTRA SUPER TEMPORARY
        static MeshHotData* GetMeshHot(MeshHandle handle) {
            return sResourceManager->GetMeshHot(handle);
        }

        // NOTE: THIS IS EXTRA SUPER TEMPORARY
        static GPUBufferHotData* GetGPUBufferHot(GPUBufferHandle handle) {
            return sResourceManager->GetGPUBufferHot(handle);
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

        static void SetDrawPassData(DrawPassHandle handle, void* data, uint32_t size) {
            sRenderer->SetDrawPassData(handle, data, size);
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
