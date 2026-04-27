#include <Engine.h>

#include <ImGui/Backend/ImGuiGLFW.h>
#include <ImGui/Backend/ImGuiVulkan.h>
#include <imgui.h>

#include <cassert>
#include <memory>

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

        mVulkanRenderer->OnImGui();

        ImGui::PopStyleVar();
        ImGui::Render();

        mVulkanRenderer->Render();
    }
}
