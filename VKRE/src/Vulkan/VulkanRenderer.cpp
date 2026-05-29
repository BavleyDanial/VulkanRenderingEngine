#include <Vulkan/VulkanRenderer.h>

#include <Core/Application.h>

#include <ResourceManager/ShaderCompiler.h>

#include <glm/glm.hpp>

#include <cassert>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace VKRE {

    static VkShaderStageFlagBits ToVkShaderStage(ShaderStage stage) {
        switch (stage) {
            case VKRE::ShaderStage::Vertex:                 return VK_SHADER_STAGE_VERTEX_BIT;
            case VKRE::ShaderStage::Fragment:               return VK_SHADER_STAGE_FRAGMENT_BIT;
            case VKRE::ShaderStage::Compute:                return VK_SHADER_STAGE_COMPUTE_BIT;
            case VKRE::ShaderStage::None:                   return VK_SHADER_STAGE_ALL; // TODO: Make this different because this is stupid
        }
    };

    VulkanRenderer::VulkanRenderer(VulkanContext& context, ResourceManager& resourceManager)
    :mContext(context), mResourceManager(resourceManager) {
        Application::GetInstance().GetEventDispatcher().RegisterListener<WindowResizeEvent>(this, &VulkanRenderer::ReSize);

        mResourceCache = std::make_unique<VulkanResourceCache>(mContext, mResourceManager);
        mUploader = std::make_unique<VulkanUploader>(mContext, *mResourceCache);
        mFrameManager = std::make_unique<VulkanFrameManager>(mContext);
        mPresenter = std::make_unique<VulkanPresenter>(mContext);

        CreateDrawImage();
        InitDescriptors();
        InitPasses();
    }

    VulkanRenderer::~VulkanRenderer() {
        vkDeviceWaitIdle(mContext.GetLogicalDevice().handle);

        mDrawPasses.clear();
        mComputePasses.clear();
        mImGuiPass.reset();

        mPresenter.reset();
        mFrameManager.reset();

        mUploader.reset();
        mDrawImage.reset();
        mResourceCache->DestroyAll();

        mGlobalDescriptorAllocator.DestroyPool(mContext.GetLogicalDevice().handle);
        vkDestroyDescriptorSetLayout(mContext.GetLogicalDevice().handle, mDrawImageDescriptorLayout, nullptr);
    }

    void VulkanRenderer::UploadMesh(GPUBufferHandle VertexBuffer, GPUBufferHandle IndexBuffer, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        mResourceCache->AllocateBuffer(VertexBuffer);
        mResourceCache->AllocateBuffer(IndexBuffer);

        mUploader->Begin();
        mUploader->UploadBuffer(VertexBuffer, vertices.data(), vertices.size() * sizeof(Vertex), 0);
        mUploader->UploadBuffer(IndexBuffer, indices.data(), indices.size() * sizeof(uint32_t), 0);
        mUploader->End();
    }

    uint64_t VulkanRenderer::GetBufferDeviceAddress(GPUBufferHandle buffer) {
        return mResourceCache->GetBufferData(buffer)->deviceAddress;
    }

    DrawPassHandle VulkanRenderer::AddDrawPass(const DrawPassDesc& desc) {
        ShaderLoadingResults shaderResults = ShaderCompiler::LoadFromFile(mResourceManager, desc.shaderPath);
        if (!shaderResults.Succeeded()) {
            std::println("VulkanRenderer::AddDrawPass Failed to load {}", desc.shaderPath);
            return INVALID_DRAW_PASS;
        }

        ResourceRef<ShaderTag> vertexShader = shaderResults.GetShader(ShaderStage::Vertex);
        if (!vertexShader) {
            std::println("VulkanRenderer::AddDrawPass {} shader has no vertex stage", desc.shaderPath);
            return INVALID_DRAW_PASS;
        }

        ResourceRef<ShaderTag> fragmentShader = shaderResults.GetShader(ShaderStage::Fragment);
        if (!fragmentShader) {
            std::println("VulkanRenderer::AddDrawPass {} shader has no fragment stage", desc.shaderPath);
            return INVALID_DRAW_PASS;
        }

        if (!mResourceCache->CreateShader(vertexShader.Get())) {
            std::println("VulkanRenderer::AddDrawPass Failed to upload {} vertex stage", desc.shaderPath);
            return INVALID_DRAW_PASS;
        }

        if (!mResourceCache->CreateShader(fragmentShader.Get())) {
            std::println("VulkanRenderer::AddDrawPass Failed to upload {} fragment stage", desc.shaderPath);
            return INVALID_DRAW_PASS;
        }

        VulkanPipelineLayoutKey layoutKey {
            .descriptorSetLayouts = { mDrawImageDescriptorLayout },
            .pushConstantRanges = desc.pushConstantRanges
        };

        VkPipelineLayout pipelineLayout = mResourceCache->CreatePipelineLayout(layoutKey);
        if (!pipelineLayout) {
            std::println("VulkanRenderer::AddDrawPass Failed to create or retreive pipeline layout");
            return INVALID_DRAW_PASS;
        }

        VulkanGraphicsPipelineKey pipelineKey {};
        pipelineKey.vertexShader = vertexShader.Get();
        pipelineKey.fragmentShader = fragmentShader.Get();
        pipelineKey.layout = pipelineLayout;
        pipelineKey.colorAttachmentFromats = desc.colorAttachmentFormats;
        pipelineKey.depthAttachmentFormat = desc.depthAttachmentFormat;
        pipelineKey.stencilAttachmentFormat = desc.stencilAttachmentFormat;
        pipelineKey.depthTestEnable = VK_TRUE;
        pipelineKey.depthWriteEnable = VK_TRUE;
        pipelineKey.depthCompareOp = VK_COMPARE_OP_LESS;

        if (!mResourceCache->CreateGraphicsPipeline(pipelineKey)) {
            std::println("VulkanRenderer::AddDrawPass Failed to create or retreive pipeline");
            return INVALID_DRAW_PASS;
        }

        VkShaderStageFlags pushConstantsShaderStages = 0;
        for (const auto& stage : desc.pushConstantRanges)
            pushConstantsShaderStages |= stage.stageFlags;

        mDrawPasses.emplace_back(*mResourceCache, pipelineKey, mDrawImageDescriptors, pushConstantsShaderStages);
        return static_cast<DrawPassHandle>(mDrawPasses.size() - 1);
    }

    void VulkanRenderer::SubmitMeshDraw(DrawPassHandle handle, const MeshDrawCommand& cmd) {
        if (handle == INVALID_DRAW_PASS || handle >= mDrawPasses.size()) {
            std::println("VulkanRenderer::SubmitMeshDraw Invalid draw pass handle");
            return;
        }
        mDrawPasses[handle].SubmitDraw(cmd);
    }

    ComputePassHandle VulkanRenderer::AddComputePass(const ComputePassDesc& desc) {
        ShaderLoadingResults shaderResults = ShaderCompiler::LoadFromFile(mResourceManager, desc.shaderPath);
        if (!shaderResults.Succeeded()) {
            std::println("VulkanRenderer::AddComputePass Failed to load {}", desc.shaderPath);
            return INVALID_COMPUTE_PASS;
        }

        ResourceRef<ShaderTag> shader = shaderResults.GetShader(ShaderStage::Compute);
        if (!shader.IsValid()) {
            std::println("VulkanRenderer::AddComputePass  {} shader has no compute stage", desc.shaderPath);
            return INVALID_COMPUTE_PASS;
        }

        if (!mResourceCache->CreateShader(shader.Get())) {
            std::println("VulkanRenderer::AddComputePass Failed to upload {}", desc.shaderPath);
            return INVALID_COMPUTE_PASS;
        }

        VulkanPipelineLayoutKey layoutKey {
            .descriptorSetLayouts = { mDrawImageDescriptorLayout },
            .pushConstantRanges = desc.pushConstantRanges
        };

        VkPipelineLayout pipelineLayout = mResourceCache->CreatePipelineLayout(layoutKey);
        if (!pipelineLayout) {
            std::println("VulkanRenderer::AddComputePass Failed to create or retreive pipeline layout");
            return INVALID_COMPUTE_PASS;
        }

        VulkanComputePipelineKey pipelineKey { shader.Get(), pipelineLayout };
        if (!mResourceCache->CreateComputePipeline(pipelineKey)) {
            std::println("VulkanRenderer::AddComputePass Failed to create or retreive pipeline");
            return INVALID_COMPUTE_PASS;
        }

        mComputePasses.emplace_back(*mResourceCache, pipelineKey, mDrawImageDescriptors, glm::vec3(desc.workgroupX, desc.workgroupY, desc.workgroupZ));
        return static_cast<ComputePassHandle>(mComputePasses.size() - 1);
    }

    void VulkanRenderer::SetComputePassData(ComputePassHandle handle, const void* data, uint32_t size) {
        if (handle == INVALID_COMPUTE_PASS || handle >= mComputePasses.size()) {
            std::println("VulkanRenderer::SetComputePassData Invalid compute pass handle");
            return;
        }
        mComputePasses[handle].SetPushConstantData(data, size);
    }

    void VulkanRenderer::CreateDrawImage() {
        auto [width, height] = mPresenter->GetSwapChain().extent;
        VkExtent3D drawImageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
        VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
        VkImageUsageFlags drawImageUsages{};
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VmaAllocationCreateInfo drawImageAllocInfo = {};
        drawImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        drawImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        mDrawImage = std::make_unique<VulkanImage2D>(mContext);
        mDrawImage->ReCreateImage(format, drawImageUsages, drawImageExtent, VK_IMAGE_ASPECT_COLOR_BIT, drawImageAllocInfo);

        mDepthImage = std::make_unique<VulkanImage2D>(mContext);
        mDepthImage->ReCreateImage(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, drawImageExtent, VK_IMAGE_ASPECT_DEPTH_BIT, drawImageAllocInfo);
    }

    void VulkanRenderer::ReCreateDrawImage() {
        mGlobalDescriptorAllocator.ClearDescriptors(mContext.GetLogicalDevice().handle);
        CreateDrawImage();
        InitDrawImageDescriptor();

        for (auto& pass : mComputePasses)
            pass.ReBuild(mDrawImageDescriptors);

        for (auto& pass : mDrawPasses)
            pass.ReBuild(mDrawImageDescriptors);
    }

    void VulkanRenderer::InitPasses() {
        mImGuiPass = std::make_unique<VulkanImGuiPass>(mContext, *mPresenter);
    }

    void VulkanRenderer::InitDescriptors() {
        std::vector<DescriptorAllocator::PoolSizeRatio> poolSizes = {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
        };

        mGlobalDescriptorAllocator.InitPool(mContext.GetLogicalDevice().handle, 10, poolSizes);

        {   // Compute Descriptor Set
            DescriptorLayoutBuilder layoutBuilder;
            layoutBuilder.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0);
            mDrawImageDescriptorLayout = layoutBuilder.Build(mContext.GetLogicalDevice().handle, VK_SHADER_STAGE_COMPUTE_BIT);
        }

        InitDrawImageDescriptor();
    }

    void VulkanRenderer::InitDrawImageDescriptor() {
        mDrawImageDescriptors = mGlobalDescriptorAllocator.Allocate(mContext.GetLogicalDevice().handle, mDrawImageDescriptorLayout);

        VkDescriptorImageInfo imgInfo{};
        imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imgInfo.imageView = mDrawImage->GetImageInfo().imageView;

        VkWriteDescriptorSet drawImageWrite = {};
        drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        drawImageWrite.pNext = nullptr;

        drawImageWrite.dstBinding = 0;
        drawImageWrite.dstSet = mDrawImageDescriptors;
        drawImageWrite.descriptorCount = 1;
        drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        drawImageWrite.pImageInfo = &imgInfo;

        vkUpdateDescriptorSets(mContext.GetLogicalDevice().handle, 1, &drawImageWrite, 0, nullptr);
    }

    void VulkanRenderer::Render() {
        VulkanFrameData& frame = mFrameManager->GetCurrentFrame();
        VK_CHECK(vkWaitForFences(mContext.GetLogicalDevice().handle, 1, &frame.waitFence, true, UINT64_MAX));

        uint32_t swapchainImageIndex = 0;
        VkResult acquireResult = vkAcquireNextImageKHR(mContext.GetLogicalDevice().handle, mPresenter->GetSwapChain().handle, UINT64_MAX, frame.presentCompleteSemaphore, nullptr, &swapchainImageIndex);
        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
            auto [width, height] = Application::GetInstance().GetWindow().GetFrameBufferExtents();
            ReSize({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
            VK_CHECK(vkResetFences(mContext.GetLogicalDevice().handle, 1, &frame.waitFence));
            return;
        }
        VK_CHECK(acquireResult);
        VK_CHECK(vkResetFences(mContext.GetLogicalDevice().handle, 1, &frame.waitFence));

        // NOTE: The following is temporary!
        VkCommandBuffer cmd = frame.commandBuffer;
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfo.pNext = nullptr;
        cmdBufferBeginInfo.pInheritanceInfo = nullptr;
        cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBufferBeginInfo));

        VkExtent2D drawImageExtent = {
            mDrawImage->GetImageInfo().extent.width,
            mDrawImage->GetImageInfo().extent.height
        };

        ImageUtils::TransitionImage(cmd, mDrawImage->GetImageInfo().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        // TODO: Replace this with passes
        ClearImage(cmd);

        for (auto& pass : mComputePasses) {
            if (pass.IsActive())
                pass.Execute(cmd, drawImageExtent);
        }

        ImageUtils::TransitionImage(cmd, mDrawImage->GetImageInfo().image,VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkClearValue clearValue{};
        clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
        VkClearValue clearDepthValue{};
        clearDepthValue.depthStencil = {1.0, 0};

        VkRenderingAttachmentInfo colorAttachment = VulkanUtils::AttatchmentInfo(mDrawImage->GetImageInfo().imageView, &clearValue , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = mDepthImage->GetImageInfo().imageView;
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.clearValue.depthStencil = clearDepthValue.depthStencil;

        ImageUtils::TransitionImage(cmd, mDepthImage->GetImageInfo().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        RenderTargetInfo targetInfo{};
        targetInfo.colorAttachments = { colorAttachment };
        targetInfo.depthAttachment = &depthAttachment;

        for (auto& pass : mDrawPasses) {
            if (pass.IsActive())
                pass.Execute(cmd, drawImageExtent, targetInfo);
        }

        ImageUtils::TransitionImage(cmd, mDrawImage->GetImageInfo().image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        VkImage swapChainImage = mPresenter->GetImages()[swapchainImageIndex];
        ImageUtils::TransitionImage(cmd, swapChainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        ImageUtils::CopyImage(cmd, mDrawImage->GetImageInfo().image, swapChainImage, drawImageExtent, mPresenter->GetSwapChain().extent);
        ImageUtils::TransitionImage(cmd, swapChainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        mImGuiPass->Execute(cmd, { swapchainImageIndex, nullptr} ); // Execute ImGuiPass

        ImageUtils::TransitionImage(cmd, swapChainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        VK_CHECK(vkEndCommandBuffer(cmd));

        VkSemaphoreSubmitInfo presentCompleteSemaphoreSubmitInfo{};
        presentCompleteSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        presentCompleteSemaphoreSubmitInfo.semaphore = frame.presentCompleteSemaphore;
        presentCompleteSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;

        VkSemaphoreSubmitInfo renderCompleteSemaphoreSubmitInfo{};
        renderCompleteSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        renderCompleteSemaphoreSubmitInfo.semaphore = mPresenter->GetRenderCompleteSemaphore(swapchainImageIndex);
        renderCompleteSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

        VkCommandBufferSubmitInfo cmdSubmitInfo;
        cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdSubmitInfo.pNext = nullptr;
        cmdSubmitInfo.commandBuffer = cmd;
        cmdSubmitInfo.deviceMask = 0;

        VkSubmitInfo2 info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        info.pNext = nullptr;
        info.waitSemaphoreInfoCount = 1;
        info.pWaitSemaphoreInfos = &presentCompleteSemaphoreSubmitInfo;
        info.signalSemaphoreInfoCount = 1;
        info.pSignalSemaphoreInfos = &renderCompleteSemaphoreSubmitInfo;
        info.commandBufferInfoCount = 1;
        info.pCommandBufferInfos = &cmdSubmitInfo;

        VK_CHECK(vkQueueSubmit2(mContext.GetGraphicsQueue(), 1, &info, frame.waitFence));
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.pSwapchains = &mPresenter->GetSwapChain().handle;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &mPresenter->GetRenderCompleteSemaphore(swapchainImageIndex);
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &swapchainImageIndex;
        VK_CHECK(vkQueuePresentKHR(mContext.GetGraphicsQueue(), &presentInfo));

        mFrameManager->AdvanceFrame();
    }

    void VulkanRenderer::OnImGui() {}

    void VulkanRenderer::ReSize(const WindowResizeEvent& event) {
        vkDeviceWaitIdle(mContext.GetLogicalDevice().handle);
        mPresenter->ResizeSwapChain(event.Width, event.Height);
        ReCreateDrawImage();
    }

    glm::vec2 VulkanRenderer::GetViewportDimensions() const {
        return {
            mDrawImage->GetImageInfo().extent.width,
            mDrawImage->GetImageInfo().extent.height
        };
    }

    void VulkanRenderer::ClearImage(VkCommandBuffer cmd) {
        VkClearColorValue clearValue;
        clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        VkImageSubresourceRange clearRange = ImageUtils::ImageSubSourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
        vkCmdClearColorImage(cmd, mDrawImage->GetImageInfo().image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
    }

}
