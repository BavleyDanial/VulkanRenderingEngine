#include <Application.h>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ImGui/Backend/ImGuiGLFW.h>
#include <ImGui/Backend/ImGuiVulkan.h>

#include <Renderer.h>

namespace VKRE {

    Application::Application() {
        if (mInstance) {
            assert("Application has already been initialised!");
        }

        mInstance = this;

        mWindow = std::make_shared<VKRE::Window>(VKRE::WindowSpecs{ .resizable = true });
        mResourceManager = std::make_unique<VKRE::ResourceManager>();
        mVulkanContext = std::make_unique<VKRE::VulkanContext>(mWindow);
        mVulkanRenderer = std::make_unique<VKRE::VulkanRenderer>(*mVulkanContext, *mResourceManager);
        Renderer::SetRenderer(mVulkanRenderer.get());
    }

    Application::~Application() {
        mVulkanRenderer.reset();
        mVulkanContext.reset();
        mResourceManager.reset();
        mWindow.reset();
    }

    void Application::Run() {
        // TODO: Change this to close when the engine decides to close, not when ONE WINDOW decides it's done. This will help with multiple windows as well.
        while (!mWindow->ShouldClose()) {
            mWindow->OnUpdate();

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            for (auto& layer : mLayersStack)
                layer->OnUpdate(0);

            for (auto& layer : mLayersStack)
                layer->OnUIRender();

            mVulkanRenderer->OnImGui();
            ImGui::Render();
            mVulkanRenderer->Render();
        }
    }

}
