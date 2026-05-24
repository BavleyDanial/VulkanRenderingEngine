#include <Vulkan/VulkanResourceCache.h>

#include <Vulkan/VulkanGPUBuffer.h>
#include <Vulkan/VulkanPipeline.h>
#include <Vulkan/VulkanUtils.h>
#include <Vulkan/VulkanPipelineBuilder.h>

#include <ResourceManager/Resources.h>

//NOTE: SUEPR EXTRA TEMPORARY
#include <Renderer/Renderer.h>
#include <cassert>
#include <print>

namespace VKRE {

    VulkanResourceCache::VulkanResourceCache(VulkanContext& context, ResourceManager& manager)
        :mContext(context), mResourceManager(manager) {}

    bool VulkanResourceCache::AllocateBuffer(GPUBufferHandle handle) {
        if (!handle.IsValid()) {
            std::println("VulkanResourceCache::UploadBuffer handle is invalid");
            return false;
        }

        auto it = mBuffers.find(handle);
        if (it != mBuffers.end()) return true;

        GPUBufferHotData* hot = mResourceManager.GetGPUBufferHot(handle);
        assert(hot && "VulkanResourceCache::UploadBuffer buffer handle is valid but GetGPUBufferHot returned nullptr");

        const GPUBufferColdData* cold = mResourceManager.GetGPUBufferCold(handle);
        assert(hot && "VulkanResourceCache::UploadBuffer buffer handle is valid but GetGPUBufferCold returned nullptr");

        VkBufferUsageFlags vkUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        if ((cold->Usage & GPUBufferUsage::Vertex) == GPUBufferUsage::Vertex)           vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if ((cold->Usage & GPUBufferUsage::Index) == GPUBufferUsage::Index)             vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if ((cold->Usage & GPUBufferUsage::Uniform) == GPUBufferUsage::Uniform)         vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if ((cold->Usage & GPUBufferUsage::Storage) == GPUBufferUsage::Storage)         vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if ((cold->Usage & GPUBufferUsage::Indirect) == GPUBufferUsage::Indirect)       vkUsage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        if ((cold->Usage & GPUBufferUsage::TransferSrc) == GPUBufferUsage::TransferSrc) vkUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if ((cold->Usage & GPUBufferUsage::TransferDst) == GPUBufferUsage::TransferDst) vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo{};
        if (hot->HostVisible) {
            allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        } else {
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        VulkanGPUBuffer buffer(mContext);
        buffer.CreateBuffer(hot->Size, vkUsage, allocInfo);

        hot->DeviceAddress = buffer.GetGPUBufferInfo().deviceAddress;
        mBuffers.emplace(handle, std::move(buffer));
        return true;
    }

    VulkanGPUBufferData* VulkanResourceCache::GetBufferData(GPUBufferHandle handle) {
        auto it = mBuffers.find(handle);
        if (it == mBuffers.end()) return nullptr;
        return &it->second.GetGPUBufferInfo();
    }

    const VulkanGPUBufferData* VulkanResourceCache::GetBufferData(GPUBufferHandle handle) const {
        auto it = mBuffers.find(handle);
        if (it == mBuffers.end()) return nullptr;
        return &it->second.GetGPUBufferInfo();
    }

    bool VulkanResourceCache::IsBufferAllocated(GPUBufferHandle handle) const {
        return mBuffers.contains(handle);
    }

    void VulkanResourceCache::DesroyBuffer(GPUBufferHandle handle) {
        auto it = mBuffers.find(handle);
        if (it == mBuffers.end()) return;
        it->second.Release();
    }

    void VulkanResourceCache::DestroyAllBuffers() {
        for (auto& [handle, buffer] : mBuffers)
            buffer.Release();
        mBuffers.clear();
    }

    bool VulkanResourceCache::CreateShader(ShaderHandle handle) {
        if (!handle.IsValid()) {
            std::println("VulkanResourceCache::CreateShader handle is invalid");
            return false;
        }

        auto it = mShaderModules.find(handle);
        bool alreadyUploaded = it != mShaderModules.end();

        ShaderColdData* cold = mResourceManager.GetShaderCold(handle);
        assert(cold && "VulkanResourceCache::CreateShader shader is valid but GetShaderCold returned nullptr");

        if (alreadyUploaded) {
            if (!cold->IsDirty) {
                return true;
            }

            vkDestroyShaderModule(mContext.GetLogicalDevice().handle, it->second, nullptr);
            mShaderModules.erase(it);
        }

        return CreateShaderModule(handle, cold);
    }

    VkShaderModule VulkanResourceCache::GetShaderModule(ShaderHandle handle) const {
        auto it = mShaderModules.find(handle);
        if (it == mShaderModules.end()) return nullptr;
        return it->second;
    }

    VkShaderModule VulkanResourceCache::GetShaderModule(ShaderHandle handle) {
        auto it = mShaderModules.find(handle);
        if (it == mShaderModules.end()) return nullptr;
        return it->second;
    }

    bool VulkanResourceCache::IsShaderUploaded(ShaderHandle handle) const {
        return mShaderModules.contains(handle);
    }

    void VulkanResourceCache::SyncDirtyShaders() {
        std::vector<ShaderHandle> dirty;
        for (auto& [handle, module] : mShaderModules) {
            const ShaderColdData* cold = mResourceManager.GetShaderCold(handle);
            if (cold && cold->IsDirty)
                dirty.push_back(handle);
        }

        for (auto handle : dirty)
            CreateShader(handle);
    }

    void VulkanResourceCache::DestroyShaderIfUnused(ShaderHandle handle) {
        if (mResourceManager.IsShaderValid(handle)) return;

        auto it = mShaderModules.find(handle);
        if (it == mShaderModules.end()) return;

        vkDestroyShaderModule(mContext.GetLogicalDevice().handle, it->second, nullptr);
        mShaderModules.erase(it);
    }

    void VulkanResourceCache::DestroyAllShaders() {
        for (auto& [handle, module] : mShaderModules)
            vkDestroyShaderModule(mContext.GetLogicalDevice().handle, module, nullptr);
        mShaderModules.clear();
    }

    VkPipelineLayout VulkanResourceCache::CreatePipelineLayout(const VulkanPipelineLayoutKey& key) {
        auto it = mPipelineLayouts.find(key);
        if (it != mPipelineLayouts.end())
            return it->second;

        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts = key.descriptorSetLayouts;
        const std::vector<VkPushConstantRange>& pushConstantRanges = key.pushConstantRanges;

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.pSetLayouts = descriptorSetLayouts.empty() ? nullptr : descriptorSetLayouts.data();
        layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        layoutInfo.pPushConstantRanges = pushConstantRanges.empty() ? nullptr : pushConstantRanges.data();
        layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());

        VkPipelineLayout layout = VK_NULL_HANDLE;
        if (vkCreatePipelineLayout(mContext.GetLogicalDevice().handle, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            std::println("VulkanComputePipelineBuilder::Build Failed to create Pipeline Layout");
            return nullptr;
        }

        mPipelineLayouts[key] = std::move(layout);
        return layout;
    }

    VkPipelineLayout VulkanResourceCache::GetPipelineLayout(const VulkanPipelineLayoutKey& key) {
        auto it = mPipelineLayouts.find(key);
        if (it == mPipelineLayouts.end()) return nullptr;
        return it->second;
    }

    VkPipelineLayout VulkanResourceCache::GetPipelineLayout(const VulkanPipelineLayoutKey& key) const {
        auto it = mPipelineLayouts.find(key);
        if (it == mPipelineLayouts.end()) return nullptr;
        return it->second;
    }

    bool VulkanResourceCache::IsPipelineLayoutCreated(const VulkanPipelineLayoutKey& key) const {
        return mPipelineLayouts.contains(key);
    }

    void VulkanResourceCache::DestroyPipelineLayout(const VulkanPipelineLayoutKey& key) {
        VkPipelineLayout layout = GetPipelineLayout(key);
        if (layout) {
            vkDestroyPipelineLayout(mContext.GetLogicalDevice().handle, layout, nullptr);
            mPipelineLayouts.erase(key);
        }
    }

    void VulkanResourceCache::DestroyAllPipelineLayouts() {
        std::vector<VulkanPipelineLayoutKey> keys;
        keys.reserve(mPipelineLayouts.size());
        for (auto& [key, layout] : mPipelineLayouts)
            keys.push_back(key);

        for (auto& key : keys)
            DestroyPipelineLayout(key);
    }

    VulkanGraphicsPipeline* VulkanResourceCache::CreateGraphicsPipeline(const VulkanGraphicsPipelineKey& key) {
        auto it = mGraphicsPipelines.find(key);
        if (it != mGraphicsPipelines.end())
            return &it->second;

        VulkanGraphicsPipelineBuilder builder;
        builder.SetPipelineLayout(key.layout);

        if (key.vertexShader.IsValid()) {
            VkShaderModule shaderModule = GetShaderModule(key.vertexShader);
            if (!shaderModule) {
                std::println("VulkanResourceCache::CreateGraphicsPipeline Failed to retreive or create a shader module from shader (index={})", static_cast<uint32_t>(key.vertexShader.index));
                return nullptr;
            }

            const ShaderHotData* hot = mResourceManager.GetShaderHot(key.vertexShader);
            builder.SetVertexShader(shaderModule, hot->Entrypoint);
        }

        if (key.fragmentShader.IsValid()) {
            VkShaderModule shaderModule = GetShaderModule(key.fragmentShader);
            if (!shaderModule) {
                std::println("VulkanResourceCache::CreateGraphicsPipeline Failed to retreive or create a shader module from shader (index={})", static_cast<uint32_t>(key.fragmentShader.index));
                return nullptr;
            }

            const ShaderHotData* hot = mResourceManager.GetShaderHot(key.fragmentShader);
            builder.SetFragmentShader(shaderModule, hot->Entrypoint);
        }

        // TODO: Add other shaders

        builder.SetTopology(key.topology)
                .SetPrimitiveRestart(key.primitveRestartEnable)
                .SetPolygonMode(key.polygonMode)
                .SetCullMode(key.cullMode)
                .SetFrontFace(key.frontFace)
                .SetDepthClamp(key.depthClampEnable)
                .SetRasterDiscard(key.rasterDiscardEnable)
                .SetSamples(key.samples)
                .SetSampleShading(key.sampleShadingEnable)
                .SetDepthTest(key.depthTestEnable)
                .SetDepthWrite(key.depthWriteEnable)
                .SetDepthCompareOp(key.depthCompareOp)
                .SetStencilTest(key.stencilTestEnable)
                .SetBlendEnable(key.blendEnable)
                .SetColorBlend(key.srcColorBlendFactor, key.dstColorBlendFactor, key.colorBlendOp)
                .SetAlphaBlend(key.srcAlphaBlendFactor, key.dstAlphaBlendFactor, key.alphaBlendOp)
                .SetColorWriteMask(key.colorWriteMask)
                .SetColorAttachmentFormats(key.colorAttachmentFromats)
                .SetDepthAttachmentFormat(key.depthAttachmentFormat)
                .SetStencilAttachmentFormat(key.stencilAttachmentFormat);

        VulkanGraphicsPipeline pipeline = builder.Build(mContext.GetLogicalDevice().handle);
        if (!pipeline.Succeeded()) {
            std::println("VulkanResourceCache::CreateGraphicsPipeline Failed to create graphics pipeline");
            return nullptr;
        }

        pipeline.vertexShader = ResourceRef<ShaderTag>(key.vertexShader, &mResourceManager);
        pipeline.fragmentShader = ResourceRef<ShaderTag>(key.fragmentShader, &mResourceManager);
        pipeline.geometryShader = ResourceRef<ShaderTag>(key.geometryShader, &mResourceManager);
        pipeline.tessControlShader = ResourceRef<ShaderTag>(key.tessControlShader, &mResourceManager);
        pipeline.tessEvalShader = ResourceRef<ShaderTag>(key.tessEvalShader, &mResourceManager);

        mGraphicsPipelines[key] = std::move(pipeline);
        return &mGraphicsPipelines[key];
    }

    VulkanGraphicsPipeline* VulkanResourceCache::GetGraphicsPipeline(const VulkanGraphicsPipelineKey& key) {
        auto it = mGraphicsPipelines.find(key);
        if (it == mGraphicsPipelines.end()) return nullptr;
        return &it->second;
    }

    const VulkanGraphicsPipeline* VulkanResourceCache::GetGraphicsPipeline(const VulkanGraphicsPipelineKey& key) const {
        auto it = mGraphicsPipelines.find(key);
        if (it == mGraphicsPipelines.end()) return nullptr;
        return &it->second;
    }

    bool VulkanResourceCache::IsGraphicsPipelineCreated(const VulkanGraphicsPipelineKey& key) const {
        return mGraphicsPipelines.contains(key);
    }

    void VulkanResourceCache::DestroyGraphicsPipeline(const VulkanGraphicsPipelineKey& key) {
        VulkanGraphicsPipeline* pipeline = GetGraphicsPipeline(key);

        if (pipeline) {
            ShaderHandle vertexHandle = pipeline->vertexShader.Get();
            ShaderHandle fragmentHandle = pipeline->fragmentShader.Get();
            ShaderHandle geometryHandle = pipeline->geometryShader.Get();
            ShaderHandle tessControlHandle = pipeline->tessControlShader.Get();
            ShaderHandle tessEvalHandle = pipeline->tessEvalShader.Get();

            pipeline->Destroy(mContext.GetLogicalDevice().handle);
            mGraphicsPipelines.erase(key);

            DestroyShaderIfUnused(vertexHandle);
            DestroyShaderIfUnused(fragmentHandle);
            DestroyShaderIfUnused(geometryHandle);
            DestroyShaderIfUnused(tessControlHandle);
            DestroyShaderIfUnused(tessEvalHandle);
        }
    }

    void VulkanResourceCache::DestroyAllGraphicsPipelines() {
        std::vector<VulkanGraphicsPipelineKey> keys;
        keys.reserve(mGraphicsPipelines.size());
        for (auto& [key, pipeline] : mGraphicsPipelines)
            keys.push_back(key);

        for (auto& key : keys)
            DestroyGraphicsPipeline(key);
    }

    VulkanComputePipeline* VulkanResourceCache::CreateComputePipeline(const VulkanComputePipelineKey& key) {
        auto it = mComputePipelines.find(key);
        if (it != mComputePipelines.end())
            return &it->second;

        VkShaderModule shaderModule = GetShaderModule(key.shader);
        if (!shaderModule) {
            if (CreateShader(key.shader)) {
                shaderModule = GetShaderModule(key.shader);
            } else {
                std::println("VulkanResourceCache::CreateComputePipeline Failed to retreive or create a shader module from shader (index={})", static_cast<uint32_t>(key.shader.index));
                return nullptr;
            }
        }

        VulkanComputePipeline pipeline = VulkanComputePipelineBuilder()
            .SetComputeShader(shaderModule)
            .SetPipelineLayout(key.layout)
            .Build(mContext.GetLogicalDevice().handle);

        if (!pipeline.Succeeded()) {
            std::println("VulkanResourceCache::CreateComputePipeline Failed to create compute pipeline");
            return nullptr;
        }

        pipeline.shader = ResourceRef<ShaderTag>(key.shader, &mResourceManager);
        mComputePipelines[key] = std::move(pipeline);
        return &mComputePipelines[key];
    }

    const VulkanComputePipeline* VulkanResourceCache::GetComputePipeline(const VulkanComputePipelineKey& key) const {
        auto it = mComputePipelines.find(key);
        if (it == mComputePipelines.end()) return nullptr;
        return &it->second;
    }

    VulkanComputePipeline* VulkanResourceCache::GetComputePipeline(const VulkanComputePipelineKey& key) {
        auto it = mComputePipelines.find(key);
        if (it == mComputePipelines.end()) return nullptr;
        return &it->second;
    }

    bool VulkanResourceCache::IsComputePipelineCreated(const VulkanComputePipelineKey& key) const {
        return mComputePipelines.contains(key);
    }

    void VulkanResourceCache::DestroyComputePipeline(const VulkanComputePipelineKey& key) {
        VulkanComputePipeline* pipeline = GetComputePipeline(key);
        if (pipeline) {
            ShaderHandle handle = pipeline->shader.Get();
            pipeline->Destroy(mContext.GetLogicalDevice().handle);
            mComputePipelines.erase(key);
            DestroyShaderIfUnused(handle);
        }
    }

    void VulkanResourceCache::DestroyAllComputePipelines() {
        std::vector<VulkanComputePipelineKey> keys;
        keys.reserve(mComputePipelines.size());
        for (auto& [key, pipeline] : mComputePipelines)
            keys.push_back(key);

        for (auto& key : keys)
            DestroyComputePipeline(key);
    }

    void VulkanResourceCache::DestroyAll() {
        DestroyAllBuffers();
        DestroyAllGraphicsPipelines();
        DestroyAllComputePipelines();
        DestroyAllPipelineLayouts();
        DestroyAllShaders();
    }

    bool VulkanResourceCache::CreateShaderModule(ShaderHandle handle, ShaderColdData* cold) {
        if (cold->ByteCode.empty()) {
            std::println("VulkanResourceCache::CreateShaderModule Cannot create shader module, SPIRV is empty (index={})", static_cast<uint32_t>(handle.index));
            return false;
        }

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.codeSize = cold->ByteCode.size() * sizeof(uint32_t);
        createInfo.pCode = cold->ByteCode.data();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(mContext.GetLogicalDevice().handle, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            std::println("VulkanResourceCache::CreateShaderModule Shader module has failed (index={})", static_cast<uint32_t>(handle.index));
            return false;
        }

        mShaderModules[handle] = shaderModule;
        assert(cold && "VulkanResourceCache Shader is valid but GetShaderCold returned nullptr");
        cold->IsDirty = false;

        return true;
    }

}
