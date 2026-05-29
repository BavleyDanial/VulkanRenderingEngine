#pragma once

#include <vulkan/vulkan.h>

#include <ResourceManager/ResourceRefs.h>
#include <ResourceManager/ResourceHandles.h>

#include <bit>
#include <functional>

static inline bool operator==(const VkPushConstantRange& a, const VkPushConstantRange& b) {
    return a.stageFlags == b.stageFlags && a.size == b.size && a.offset == b.offset;
}

static inline void CombineHash(size_t& seed, size_t value) {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace VKRE {

    struct VulkanPipelineLayoutKey {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts; // TODO: Change this from vector to something better for hashing
        std::vector<VkPushConstantRange> pushConstantRanges; // TODO: Change this from vector to something better for hashing

        bool operator==(const VulkanPipelineLayoutKey&) const = default;
        bool operator!=(const VulkanPipelineLayoutKey&) const = default;

        static VulkanPipelineLayoutKey Null() { return {}; }
    };

    struct VulkanGraphicsPipelineKey {
        // Shaders
        ShaderHandle vertexShader = ShaderHandle::Null();
        ShaderHandle fragmentShader = ShaderHandle::Null();
        ShaderHandle geometryShader = ShaderHandle::Null();
        ShaderHandle tessControlShader = ShaderHandle::Null();
        ShaderHandle tessEvalShader = ShaderHandle::Null();

        VkPipelineLayout layout = VK_NULL_HANDLE;

        // Input Assembly
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 primitveRestartEnable = VK_FALSE;

        // Rasterization States
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
        VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        VkBool32 depthClampEnable = VK_FALSE;
        VkBool32 rasterDiscardEnable = VK_FALSE;

        // Multisampling
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkBool32 sampleShadingEnable = VK_FALSE;

        // Depth / Stencil
        VkBool32 depthTestEnable = VK_FALSE;
        VkBool32 depthWriteEnable = VK_FALSE;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
        VkBool32 stencilTestEnable = VK_FALSE;

        // Blending
        VkBool32 blendEnable = VK_FALSE;

        VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;

        VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;

        VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;

        // Dynamic Rendering
        std::vector<VkFormat> colorAttachmentFromats;

        VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        bool operator==(const VulkanGraphicsPipelineKey&) const = default;
    };

    struct VulkanGraphicsPipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;

        ResourceRef<ShaderTag> vertexShader = ResourceRef<ShaderTag>();
        ResourceRef<ShaderTag> fragmentShader = ResourceRef<ShaderTag>();
        ResourceRef<ShaderTag> geometryShader = ResourceRef<ShaderTag>();
        ResourceRef<ShaderTag> tessControlShader = ResourceRef<ShaderTag>();
        ResourceRef<ShaderTag> tessEvalShader = ResourceRef<ShaderTag>();

        bool Succeeded() const { return pipeline != VK_NULL_HANDLE; }

        void Destroy(VkDevice device) {
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, pipeline, nullptr);
            }
            pipeline = VK_NULL_HANDLE;
            layout = VK_NULL_HANDLE; // don't destroy this, but point to nullptr, let the cache destroy it
        }
    };

    struct VulkanComputePipelineKey {
        ShaderHandle shader = ShaderHandle::Null();
        VkPipelineLayout layout = VK_NULL_HANDLE;

        bool operator==(const VulkanComputePipelineKey&) const = default;
        bool operator!=(const VulkanComputePipelineKey&) const = default;

        static VulkanComputePipelineKey Null() { return {}; }
    };

    struct VulkanComputePipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;

        ResourceRef<ShaderTag> shader = ResourceRef<ShaderTag>();

        bool Succeeded() const { return pipeline != VK_NULL_HANDLE; }

        void Destroy(VkDevice device) {
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, pipeline, nullptr);
            }
            pipeline = VK_NULL_HANDLE;
            layout = VK_NULL_HANDLE; // don't destroy this, but point to nullptr, let the cache destroy it
        }
    };

}

namespace std {
    // TODO: This is horrible for performance
    template<>
    struct hash<VKRE::VulkanPipelineLayoutKey> {
        size_t operator()(const VKRE::VulkanPipelineLayoutKey& key) const noexcept {
            size_t result = 0;

            for (auto layout : key.descriptorSetLayouts)
                CombineHash(result, std::hash<uint64_t>{}(std::bit_cast<uint64_t>(layout)));

            for (auto& range : key.pushConstantRanges) {
                CombineHash(result, std::hash<uint32_t>{}(range.size));
                CombineHash(result, std::hash<uint32_t>{}(range.offset));
                CombineHash(result, std::hash<uint32_t>{}(range.stageFlags));
            }

            return result;
        }
    };

    template<>
    struct hash<VKRE::VulkanGraphicsPipelineKey> {
        size_t operator()(const VKRE::VulkanGraphicsPipelineKey& key) const noexcept {
            size_t result = 0;
            CombineHash(result, std::hash<VKRE::ShaderHandle>{}(key.vertexShader));
            CombineHash(result, std::hash<VKRE::ShaderHandle>{}(key.fragmentShader));
            CombineHash(result, std::hash<VKRE::ShaderHandle>{}(key.geometryShader));
            CombineHash(result, std::hash<VKRE::ShaderHandle>{}(key.tessEvalShader));
            CombineHash(result, std::hash<VKRE::ShaderHandle>{}(key.tessControlShader));
            CombineHash(result, std::hash<uint64_t>{}(reinterpret_cast<uint64_t>(key.layout)));

            CombineHash(result, std::hash<uint32_t>{}(key.topology));
            CombineHash(result, std::hash<uint32_t>{}(key.primitveRestartEnable));

            CombineHash(result, std::hash<uint32_t>{}(key.polygonMode));
            CombineHash(result, std::hash<uint32_t>{}(key.cullMode));
            CombineHash(result, std::hash<uint32_t>{}(key.frontFace));
            CombineHash(result, std::hash<uint32_t>{}(key.depthClampEnable));
            CombineHash(result, std::hash<uint32_t>{}(key.rasterDiscardEnable));

            CombineHash(result, std::hash<uint32_t>{}(key.samples));
            CombineHash(result, std::hash<uint32_t>{}(key.sampleShadingEnable));

            CombineHash(result, std::hash<uint32_t>{}(key.depthTestEnable));
            CombineHash(result, std::hash<uint32_t>{}(key.depthWriteEnable));
            CombineHash(result, std::hash<uint32_t>{}(key.depthCompareOp));
            CombineHash(result, std::hash<uint32_t>{}(key.stencilTestEnable));

            CombineHash(result, std::hash<uint32_t>{}(key.blendEnable));
            CombineHash(result, std::hash<uint32_t>{}(key.srcColorBlendFactor));
            CombineHash(result, std::hash<uint32_t>{}(key.dstColorBlendFactor));
            CombineHash(result, std::hash<uint32_t>{}(key.colorBlendOp));
            CombineHash(result, std::hash<uint32_t>{}(key.srcAlphaBlendFactor));
            CombineHash(result, std::hash<uint32_t>{}(key.dstAlphaBlendFactor));
            CombineHash(result, std::hash<uint32_t>{}(key.alphaBlendOp));
            CombineHash(result, std::hash<uint32_t>{}(key.colorWriteMask));

            for (auto fmt : key.colorAttachmentFromats)
                CombineHash(result, std::hash<uint32_t>{}(fmt));

            CombineHash(result, std::hash<size_t>{}(key.colorAttachmentFromats.size()));
            CombineHash(result, std::hash<uint32_t>{}(key.depthAttachmentFormat));
            CombineHash(result, std::hash<uint32_t>{}(key.stencilAttachmentFormat));

            return result;
        }
    };

    template<>
    struct hash<VKRE::VulkanComputePipelineKey> {
        size_t operator()(const VKRE::VulkanComputePipelineKey& key) const noexcept {
            size_t result = 0;
            CombineHash(result, std::hash<VKRE::ShaderHandle>{}(key.shader));
            CombineHash(result, std::hash<uint64_t>{}(std::bit_cast<uint64_t>(key.layout)));
            return result;
        }
    };
}
