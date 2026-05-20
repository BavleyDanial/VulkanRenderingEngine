#include "Vulkan/VulkanPipeline.h"
#include <Vulkan/VulkanResourceCache.h>
#include <Vulkan/VulkanPipelineBuilder.h>

#include <cassert>
#include <print>

namespace VKRE {

    VulkanResourceCache::VulkanResourceCache(VulkanContext& context, ResourceManager& manager)
        :mContext(context), mResourceManager(manager) {}

    bool VulkanResourceCache::CreateShader(ShaderHandle handle) {
        if (!mResourceManager.IsShaderValid(handle)) {
            std::println("VulkanResourceCache::UploadShader handle is invalid");
            return false;
        }

        const ShaderHotData* hot = mResourceManager.GetShaderHot(handle);
        assert(hot && "VulkanResourceCache Shader is valid but GetShaderHot returned nullptr");

        auto it = mShaderModules.find(handle);
        bool alreadyUploaded = it != mShaderModules.end();

        ShaderColdData* cold = mResourceManager.GetShaderCold(handle);
        assert(cold && "VulkanResourceCache Shader is valid but GetShaderCold returned nullptr");

        if (alreadyUploaded && !cold->isDirty) {
            mShaderModulesRefCount[handle] += 1;
            return true;
        }

        if (alreadyUploaded)
            DestroyShader(handle);

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
            if (cold && cold->isDirty)
                dirty.push_back(handle);
        }

        for (ShaderHandle handle : dirty)
            CreateShader(handle);
    }

    void VulkanResourceCache::DestroyShader(ShaderHandle handle) {
        auto it = mShaderModules.find(handle);
        if (it == mShaderModules.end()) return;

        auto refIt = mShaderModulesRefCount.find(handle);
        if (refIt == mShaderModulesRefCount.end()) return;

        mShaderModulesRefCount[handle]--;
        if (mShaderModulesRefCount[handle] == 0) {
            vkDestroyShaderModule(mContext.GetLogicalDevice().handle, it->second, nullptr);
            mShaderModulesRefCount.erase(refIt);
            mShaderModules.erase(it);
        }
        mResourceManager.DestroyShaderRef(handle);
    }

    void VulkanResourceCache::DestroyAllShaders() {
        std::vector<ShaderHandle> handles;
        handles.reserve(mShaderModules.size());
        for (auto& [handle, module] : mShaderModules)
            handles.push_back(handle);

        for (ShaderHandle handle : handles) {
            uint32_t refCount = mShaderModulesRefCount[handle];
            for (uint32_t i = 0; i < refCount; i++)
                DestroyShader(handle);
        }
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

        mPipelineLayouts[key] = layout;
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

    VulkanComputePipeline* VulkanResourceCache::CreateComputePipeline(const VulkanComputePipelineKey& key) {
        auto it = mComputePipelines.find(key);
        if (it != mComputePipelines.end())
            return &it->second;

        VkShaderModule shaderModule = GetShaderModule(key.shader);
        if (!shaderModule) {
            if (CreateShader(key.shader)) {
                shaderModule = GetShaderModule(key.shader);
            } else {
                std::println("VulkanResourceCache::CreateComputePipeline Failed to retreive or create a shader module from shader (index={})", key.shader.index);
                return nullptr;
            }
        }

        VulkanComputePipeline pipeline = VulkanComputePipelineBuilder()
            .SetShaderModule(shaderModule)
            .SetPipelineLayout(key.layout)
            .Build(mContext.GetLogicalDevice().handle);

        if (!pipeline.Succeeded()) {
            std::println("VulkanResourceCache::CreateComputePipeline Failed to create compute pipeline");
            return nullptr;
        }

        mComputePipelines[key] = pipeline;
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
            pipeline->Destroy(mContext.GetLogicalDevice().handle);
            mComputePipelines.erase(key);

            bool isShaderStillInUse = false;
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
        DestroyAllComputePipelines();
        DestroyAllPipelineLayouts();
        DestroyAllShaders();
    }

    bool VulkanResourceCache::CreateShaderModule(ShaderHandle handle, ShaderColdData* cold) {
        if (cold->byteCode.empty()) {
            std::println("VulkanResourceCache::CreateShaderModule Cannot create shader module, SPIRV is empty (index={})", static_cast<uint32_t>(handle.index));
            return false;
        }

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.codeSize = cold->byteCode.size() * sizeof(uint32_t);
        createInfo.pCode = cold->byteCode.data();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(mContext.GetLogicalDevice().handle, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            std::println("VulkanResourceCache::CreateShaderModule Shader module has failed (index={})", static_cast<uint32_t>(handle.index));
            return false;
        }

        mShaderModules[handle] = shaderModule;
        mShaderModulesRefCount[handle] = 1;
        assert(cold && "VulkanResourceCache Shader is valid but GetShaderCold returned nullptr");
        cold->isDirty = false;

        return true;
    }

}
