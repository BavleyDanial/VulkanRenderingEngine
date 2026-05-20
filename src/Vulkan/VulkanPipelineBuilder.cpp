#include <Vulkan/VulkanPipelineBuilder.h>
#include <print>
#include <vulkan/vulkan_core.h>

namespace VKRE {

    // VULKAN GRAPHICS PIPELINE BUILDER

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetPipelineLayout(VkPipelineLayout pipelineLayout) {
        mPipelineLayout = pipelineLayout;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetFlags(VkPipelineCreateFlags flags) {
        mFlags |= flags;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetNext(void* pNext) {
        mPNext = pNext;
        return *this;
    }


    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetVertexShader(VkShaderModule shaderModule, const char* entrypoint) {
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.pNext = nullptr;
        stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stageInfo.module = shaderModule;
        stageInfo.pName = entrypoint;

        mShaderStagesInfo.push_back(stageInfo);
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetFragmentShader(VkShaderModule shaderModule, const char* entrypoint) {
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.pNext = nullptr;
        stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stageInfo.module = shaderModule;
        stageInfo.pName = entrypoint;

        mShaderStagesInfo.push_back(stageInfo);
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetGeometryShader(VkShaderModule shaderModule, const char* entrypoint) {
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.pNext = nullptr;
        stageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        stageInfo.module = shaderModule;
        stageInfo.pName = entrypoint;

        mShaderStagesInfo.push_back(stageInfo);
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetTessellationShaders(VkShaderModule controlModule, VkShaderModule evalModule, const char* entrypoint) {
        VkPipelineShaderStageCreateInfo constrolStageInfo{};
        constrolStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        constrolStageInfo.pNext = nullptr;
        constrolStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        constrolStageInfo.module = controlModule;
        constrolStageInfo.pName = entrypoint;

        VkPipelineShaderStageCreateInfo evalStageInfo{};
        evalStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        evalStageInfo.pNext = nullptr;
        evalStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        evalStageInfo.module = evalModule;
        evalStageInfo.pName = entrypoint;

        mShaderStagesInfo.push_back(constrolStageInfo);
        mShaderStagesInfo.push_back(evalStageInfo);
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindings, const std::vector<VkVertexInputAttributeDescription>& attributes) {
        mBindings.append_range(bindings);
        mAttributes.append_range(attributes);
        return *this;
    }


    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetTopology(VkPrimitiveTopology topology) {
        mTopology = topology;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetPrimitiveRestart(bool enabled) {
        mPrimitiveRestart = enabled;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetCullMode(VkCullModeFlags cullMode) {
        mCullMode |= cullMode;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetFrontFace(VkFrontFace frontFace) {
        mFrontFace = frontFace;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetPolygonMode(VkPolygonMode mode) {
        mPolygonMode = mode;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetRasterDiscard(bool enabled) {
        mRasterDiscardEnable = enabled;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetDepthClamp(bool enabled) {
        mDepthClampEnable = enabled;
        return *this;
    }


    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetDepthTest(bool enabled) {
        mDepthTest = enabled;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetDepthWrite(bool enabled) {
        mDepthWrite = enabled;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetDepthCompareOp(VkCompareOp op) {
        mDepthCompOp = op;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetStencilTest(bool enabled) {
        mStencilTest = enabled;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetBlendEnable(bool enabled) {
        mBlendEnable = enabled;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetColorBlend(VkBlendFactor src, VkBlendFactor dst, VkBlendOp op) {
        mSrcColorBlendFactor = src;
        mDstColorBlendFactor = dst;
        mColorBlendOp = op;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetAlphaBlend(VkBlendFactor src, VkBlendFactor dst, VkBlendOp op) {
        mSrcAlphaBlendFactor = src;
        mDstAlphaBlendFactor = dst;
        mAlphaBlendOp = op;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetColorWriteMask(VkColorComponentFlags mask) {
        mColorWriteMask |= mask;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetColorAttachmentFormats(const std::vector<VkFormat>& formats) {
        mColorFormats.append_range(formats);
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetDepthAttachmentFormat(VkFormat format) {
        mDepthFormat = format;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetStencilAttachmentFormat(VkFormat format) {
        mStencilFormat = format;
        return *this;
    }


    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetSamples(VkSampleCountFlagBits samples) {
        mSamples = samples;
        return *this;
    }

    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetSampleShading(bool enabled) {
        mSampleShadingEnable = enabled;
        return *this;
    }

    VulkanGraphicsPipeline VulkanGraphicsPipelineBuilder::Build(VkDevice device, VkPipelineCache cache) {
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.pNext = nullptr;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.pNext = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(mAttributes.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(mBindings.size());
        vertexInputInfo.pVertexAttributeDescriptions = static_cast<uint32_t>(mAttributes.size()) ? mAttributes.data() : nullptr;
        vertexInputInfo.pVertexBindingDescriptions = static_cast<uint32_t>(mBindings.size()) ? mBindings.data() : nullptr;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = mTopology;
        inputAssemblyInfo.primitiveRestartEnable = mPrimitiveRestart;

        VkPipelineRasterizationStateCreateInfo rasterInfo{};
        rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterInfo.pNext = nullptr;
        rasterInfo.depthClampEnable = mDepthClampEnable;
        rasterInfo.rasterizerDiscardEnable = mRasterDiscardEnable;
        rasterInfo.polygonMode = mPolygonMode;
        rasterInfo.cullMode = mCullMode;
        rasterInfo.frontFace = mFrontFace;
        rasterInfo.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo msaaInfo{};
        msaaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        msaaInfo.pNext = nullptr;
        msaaInfo.rasterizationSamples = mSamples;
        msaaInfo.sampleShadingEnable = mSampleShadingEnable;
        msaaInfo.minSampleShading = 1.0f;
        msaaInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
        depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilInfo.pNext = nullptr;
        depthStencilInfo.depthTestEnable = mDepthTest;
        depthStencilInfo.depthWriteEnable = mDepthWrite;
        depthStencilInfo.depthCompareOp = mDepthCompOp;
        depthStencilInfo.stencilTestEnable = mStencilTest;

        VkPipelineColorBlendAttachmentState colorAttachmentState{};
        colorAttachmentState.colorWriteMask = mColorWriteMask;
        colorAttachmentState.srcColorBlendFactor = mSrcColorBlendFactor;
        colorAttachmentState.dstColorBlendFactor = mDstColorBlendFactor;
        colorAttachmentState.colorBlendOp = mColorBlendOp;
        colorAttachmentState.srcAlphaBlendFactor = mSrcAlphaBlendFactor;
        colorAttachmentState.dstAlphaBlendFactor = mDstAlphaBlendFactor;
        colorAttachmentState.alphaBlendOp = mAlphaBlendOp;
        colorAttachmentState.blendEnable = mBlendEnable;

        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendInfo.pNext = nullptr;
        colorBlendInfo.logicOpEnable = VK_FALSE;
        colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendInfo.attachmentCount = 1;
        colorBlendInfo.pAttachments = &colorAttachmentState;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.pNext = nullptr;
        dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicStateInfo.pDynamicStates = dynamicStates.data();

        VkPipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingInfo.pNext = mPNext;
        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(mColorFormats.size());
        renderingInfo.pColorAttachmentFormats = static_cast<uint32_t>(mColorFormats.size()) ? mColorFormats.data() : nullptr;
        renderingInfo.depthAttachmentFormat = mDepthFormat;
        renderingInfo.stencilAttachmentFormat = mStencilFormat;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &renderingInfo;
        pipelineInfo.flags = mFlags;

        pipelineInfo.stageCount = static_cast<uint32_t>(mShaderStagesInfo.size());
        pipelineInfo.pStages = static_cast<uint32_t>(mShaderStagesInfo.size()) ? mShaderStagesInfo.data() : nullptr;

        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
        pipelineInfo.pRasterizationState = &rasterInfo;
        pipelineInfo.pMultisampleState = &msaaInfo;
        pipelineInfo.pDepthStencilState = &depthStencilInfo;
        pipelineInfo.pColorBlendState = &colorBlendInfo;
        pipelineInfo.pDynamicState = &dynamicStateInfo;

        pipelineInfo.layout = mPipelineLayout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        VkPipeline pipeline = VK_NULL_HANDLE;
        if (vkCreateGraphicsPipelines(device, cache, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            std::println("VulkanGraphicsPipelineBuilder::Build Failed to create Pipeline");
            return {};
        }

        return { pipeline, mPipelineLayout };
    }

    // VULKAN COMPUTE PIPELINE BUILDER

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetPipelineLayout(VkPipelineLayout pipelineLayout) {
        mPipelineLayout = pipelineLayout;
        return *this;
    }

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetComputeShader(VkShaderModule shaderModule, const char* entrypoint) {
        mShaderModule = shaderModule;
        mEntrypoint = entrypoint;
        return *this;
    }

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetFlags(VkPipelineCreateFlags flags) {
        mFlags |= flags;
        return *this;
    }

    VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetNext(void* pNext) {
        mPNext = pNext;
        return *this;
    }

    VulkanComputePipeline VulkanComputePipelineBuilder::Build(VkDevice device, VkPipelineCache cache) {
        if (mShaderModule == VK_NULL_HANDLE) {
            std::println("VulkanComputePipelineBuilder::Build Tried to build with invalide Shader Module");
            return {};
        }

        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.pNext = nullptr;
        stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stageInfo.module = mShaderModule;
        stageInfo.pName = mEntrypoint;

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = mPNext;
        pipelineInfo.flags = mFlags;
        pipelineInfo.layout = mPipelineLayout;
        pipelineInfo.stage = stageInfo;

        VkPipeline pipeline = VK_NULL_HANDLE;
        if (vkCreateComputePipelines(device, cache, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            std::println("VulkanComputePipelineBuilder::Build Failed to create Pipeline");
            return {};
        }

        return { pipeline, mPipelineLayout };
    }

}
