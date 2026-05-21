#pragma once

#include "Vulkan/VulkanRenderer.h"

namespace VKRE {

    // NOTE: THIS IS TEMPORARY
    class Renderer {
    public:
        static void SetRenderer(VulkanRenderer* renderer) {
            sRenderer = renderer;
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
    };

}
