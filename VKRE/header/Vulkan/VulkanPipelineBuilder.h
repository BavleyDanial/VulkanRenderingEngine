#pragma once

#include "VulkanPipeline.h"
#include <vulkan/vulkan_core.h>

namespace VKRE {

    class VulkanGraphicsPipelineBuilder {
    public:
        VulkanGraphicsPipelineBuilder& SetPipelineLayout(VkPipelineLayout pipelineLayout);
        VulkanGraphicsPipelineBuilder& SetFlags(VkPipelineCreateFlags flags);
        VulkanGraphicsPipelineBuilder& SetNext(void* pNext);

        VulkanGraphicsPipelineBuilder& SetVertexShader(VkShaderModule shaderModule, const char* entrypoint = "main");
        VulkanGraphicsPipelineBuilder& SetFragmentShader(VkShaderModule shaderModule, const char* entrypoint = "main");
        VulkanGraphicsPipelineBuilder& SetGeometryShader(VkShaderModule shaderModule, const char* entrypoint = "main");
        VulkanGraphicsPipelineBuilder& SetTessellationShaders(VkShaderModule controlModule, VkShaderModule evalModule, const char* entrypoint = "main");
        VulkanGraphicsPipelineBuilder& SetVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindings, const std::vector<VkVertexInputAttributeDescription>& attributes);

        VulkanGraphicsPipelineBuilder& SetTopology(VkPrimitiveTopology topology);
        VulkanGraphicsPipelineBuilder& SetPrimitiveRestart(bool enabled);


        VulkanGraphicsPipelineBuilder& SetCullMode(VkCullModeFlags cullMode);
        VulkanGraphicsPipelineBuilder& SetFrontFace(VkFrontFace frontFace);
        VulkanGraphicsPipelineBuilder& SetPolygonMode(VkPolygonMode mode);
        VulkanGraphicsPipelineBuilder& SetRasterDiscard(bool enabled);
        VulkanGraphicsPipelineBuilder& SetDepthClamp(bool enabled);

        VulkanGraphicsPipelineBuilder& SetDepthTest(bool enabled);
        VulkanGraphicsPipelineBuilder& SetDepthWrite(bool enabled);
        VulkanGraphicsPipelineBuilder& SetDepthCompareOp(VkCompareOp op);
        VulkanGraphicsPipelineBuilder& SetStencilTest(bool enabled);

        VulkanGraphicsPipelineBuilder& SetBlendEnable(bool enabled);
        VulkanGraphicsPipelineBuilder& SetColorBlend(VkBlendFactor src, VkBlendFactor dst, VkBlendOp op);
        VulkanGraphicsPipelineBuilder& SetAlphaBlend(VkBlendFactor src, VkBlendFactor dst, VkBlendOp op);
        VulkanGraphicsPipelineBuilder& SetColorWriteMask(VkColorComponentFlags mask);

        VulkanGraphicsPipelineBuilder& SetColorAttachmentFormats(const std::vector<VkFormat>& formats);
        VulkanGraphicsPipelineBuilder& SetDepthAttachmentFormat(VkFormat format);
        VulkanGraphicsPipelineBuilder& SetStencilAttachmentFormat(VkFormat format);

        VulkanGraphicsPipelineBuilder& SetSamples(VkSampleCountFlagBits samples);
        VulkanGraphicsPipelineBuilder& SetSampleShading(bool enabled);

        VulkanGraphicsPipeline Build(VkDevice device, VkPipelineCache cache = nullptr);

    private:
        VkPipelineLayout mPipelineLayout;
        VkPipelineCreateFlags mFlags = 0;
        void* mPNext = nullptr;

        std::vector<VkPipelineShaderStageCreateInfo> mShaderStagesInfo;

        std::vector<VkVertexInputBindingDescription> mBindings;
        std::vector<VkVertexInputAttributeDescription> mAttributes;

        VkPrimitiveTopology mTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 mPrimitiveRestart = VK_FALSE;

        VkPolygonMode mPolygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags mCullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace mFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        VkBool32 mRasterDiscardEnable = VK_FALSE;
        VkBool32 mDepthClampEnable = VK_FALSE;

        VkSampleCountFlagBits mSamples = VK_SAMPLE_COUNT_1_BIT;
        VkBool32 mSampleShadingEnable = VK_FALSE;

        VkBool32 mDepthTest = VK_FALSE;
        VkBool32 mDepthWrite = VK_FALSE;
        VkCompareOp mDepthCompOp = VK_COMPARE_OP_LESS;
        VkBool32 mStencilTest = VK_FALSE;

        VkBool32 mBlendEnable = VK_FALSE;

        VkBlendFactor mSrcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor mDstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp mColorBlendOp = VK_BLEND_OP_ADD;

        VkBlendFactor mSrcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor mDstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp mAlphaBlendOp = VK_BLEND_OP_ADD;

        VkColorComponentFlags mColorWriteMask =  VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;

        std::vector<VkFormat> mColorFormats;
        VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
        VkFormat mStencilFormat = VK_FORMAT_UNDEFINED;
    };


    class VulkanComputePipelineBuilder {
    public:
        VulkanComputePipelineBuilder& SetPipelineLayout(VkPipelineLayout pipelineLayout);
        VulkanComputePipelineBuilder& SetComputeShader(VkShaderModule shaderModule, const char* entrypoint = "main");
        VulkanComputePipelineBuilder& SetFlags(VkPipelineCreateFlags flags);
        VulkanComputePipelineBuilder& SetNext(void* pNext);

        VulkanComputePipeline Build(VkDevice device, VkPipelineCache cache = nullptr);

    private:
        VkPipelineLayout mPipelineLayout;
        VkPipelineCreateFlags mFlags = 0;
        void* mPNext = nullptr;

        VkShaderModule mShaderModule = VK_NULL_HANDLE;
        const char* mEntrypoint = "";
    };

}
