#include "ImGui/Backend/ImGuiGLFW.h"
#include "ImGui/Backend/ImGuiVulkan.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <Engine.h>

#include <cassert>
#include <memory>

Engine::Engine() {
    if (mInstance) {
        assert("Engine has already been initialised!");
    }

    mInstance = this;

    mWindow = std::make_shared<VKRE::Window>(VKRE::WindowSpecs{ .resizable = true });
    mVulkanContext = std::make_shared<VKRE::VulkanContext>(mWindow);
    mVulkanRenderer = std::make_shared<VKRE::VulkanRenderer>(mVulkanContext);
}

Engine::~Engine() {
    mVulkanRenderer.reset();
    mVulkanContext.reset();
    mWindow.reset();
}

void Engine::Run() {
    // TODO: Change this to close when the engine decides to close, not when ONE WINDOW decides it's done. This will help with multiple windows as well.
    while (!mWindow->ShouldClose()) {
        mWindow->OnUpdate();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});

        if (ImGui::Begin("background")) {

            VKRE::VulkanRenderer::ComputeEffect& selected = mVulkanRenderer->backgroundEffects[mVulkanRenderer->currentBackgroundEffect];

            ImGui::Text("Selected effect: ", selected.name);

            ImGui::SliderInt("Effect Index", &mVulkanRenderer->currentBackgroundEffect,0, mVulkanRenderer->backgroundEffects.size() - 1);

            ImGui::InputFloat4("data1",(float*)& selected.data.data1);
            ImGui::InputFloat4("data2",(float*)& selected.data.data2);
            ImGui::InputFloat4("data3",(float*)& selected.data.data3);
            ImGui::InputFloat4("data4",(float*)& selected.data.data4);
        }
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Render();

        mVulkanRenderer->Render();
    }
}
