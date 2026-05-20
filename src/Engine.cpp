#include <Engine.h>

#include <imgui.h>

#include <cassert>
#include <memory>

#include <glm/gtc/type_ptr.hpp>

#include <ImGui/Backend/ImGuiVulkan.h>
#include <ImGui/Backend/ImGuiGLFW.h>
#include <vulkan/vulkan_core.h>

Engine::Engine() {
    if (mInstance) {
        assert("Engine has already been initialised!");
    }

    mInstance = this;

    mWindow = std::make_shared<VKRE::Window>(VKRE::WindowSpecs{ .resizable = true });
    mResourceManager = std::make_unique<VKRE::ResourceManager>();
    mVulkanContext = std::make_unique<VKRE::VulkanContext>(mWindow);
    mVulkanRenderer = std::make_unique<VKRE::VulkanRenderer>(*mVulkanContext, *mResourceManager);
}

Engine::~Engine() {
    mVulkanRenderer.reset();
    mVulkanContext.reset();
    mResourceManager.reset();
    mWindow.reset();
}

void Engine::Run() {
    // NOTE: This is temporary, it will be moved out

    mTrianglePass = mVulkanRenderer->AddDrawPass({
        .shaderPath = "res/shaders/colored_triangle.glsl",
        .debugName = "triangle",
        .colorAttachmentFormats = { VK_FORMAT_R16G16B16A16_SFLOAT },
        .vertexCount = 3,
    });

    mGradientPass = mVulkanRenderer->AddComputePass({
        .shaderPath = "res/shaders/gradient.glsl",
        .debugName = "gradient",
        .pushConstantRanges = {
            {
                VK_SHADER_STAGE_COMPUTE_BIT,
                0,
                sizeof(GradientParams),
            }
        }
    });

    mSkyPass = mVulkanRenderer->AddComputePass({
        .shaderPath = "res/shaders/gradient.glsl",
        .debugName = "gradient",
        .pushConstantRanges = {
            {
                VK_SHADER_STAGE_COMPUTE_BIT,
                0,
                sizeof(GradientParams),
            }
        }
    });

    mGradientParams = { .colorA = {0, 0, 0, 1}, .colorB = {0, 0, 1, 1} };
    mSkyParams = { .colorA = {0.1f, 0.2f, 0.4f, 1}, .colorB = {0, 0.1f, .2f, 1} };

    mVulkanRenderer->SetComputePassData(mGradientPass, &mGradientParams, sizeof(mGradientParams));
    mVulkanRenderer->SetComputePassData(mSkyPass, &mSkyParams, sizeof(mSkyParams));

    int index = 0;

    // TODO: Change this to close when the engine decides to close, not when ONE WINDOW decides it's done. This will help with multiple windows as well.
    while (!mWindow->ShouldClose()) {
        mWindow->OnUpdate();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::PopStyleVar();

        if (ImGui::Begin("GradientEffect")) {
            ImGui::SliderInt("Pass", &index, 0, 2);

            if (index == 0) {
                mVulkanRenderer->ActivateDrawPass(mTrianglePass);
                mVulkanRenderer->DeActivateComputePass(mGradientPass);
                mVulkanRenderer->DeActivateComputePass(mSkyPass);

            } else if (index == 1) {
                mVulkanRenderer->ActivateComputePass(mGradientPass);
                mVulkanRenderer->DeActivateComputePass(mSkyPass);

                if (ImGui::SliderFloat4("Color A", glm::value_ptr(mGradientParams.colorA), 0.0f, 1.0f) ||
                    ImGui::SliderFloat4("Color B", glm::value_ptr(mGradientParams.colorB), 0.0f, 1.0f)) {
                    mVulkanRenderer->SetComputePassData(mGradientPass, &mGradientParams, sizeof(mGradientParams));
                }

            } else if (index == 2) {
                mVulkanRenderer->ActivateComputePass(mSkyPass);
                mVulkanRenderer->DeActivateComputePass(mGradientPass);

                if (ImGui::SliderFloat4("Color A", glm::value_ptr(mSkyParams.colorA), 0.0f, 1.0f) ||
                    ImGui::SliderFloat4("Color B", glm::value_ptr(mSkyParams.colorB), 0.0f, 1.0f)) {
                    mVulkanRenderer->SetComputePassData(mSkyPass, &mSkyParams, sizeof(mSkyParams));
                }
            }

        }
        ImGui::End();


        mVulkanRenderer->OnImGui();
        ImGui::Render();
        mVulkanRenderer->Render();
    }
}
