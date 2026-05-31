#include <Core/Application.h>

#include <chrono>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Renderer/Renderer.h>

namespace VKRE {

    Application::Application() {
        if (mInstance) {
            assert("Application has already been initialised!");
        }

        mInstance = this;

        mWindow = std::make_unique<Window>(WindowSpecs{ .resizable = true });
        mResourceManager = std::make_unique<ResourceManager>();
        mAssetManager = std::make_unique<AssetManager>(*mResourceManager);
        mVulkanContext = std::make_unique<VulkanContext>(*mWindow);
        mVulkanRenderer = std::make_unique<VulkanRenderer>(*mVulkanContext, *mResourceManager);

        Renderer::SetRenderer(mVulkanRenderer.get());
    }

    Application::~Application() {
        mVulkanRenderer.reset();
        mVulkanContext.reset();
        mAssetManager.reset();
        mResourceManager.reset();
        mWindow.reset();
    }

    void Application::Run() {
        auto lastFrameTime = std::chrono::high_resolution_clock::now();

        // TODO: Change this to close when the engine decides to close, not when ONE WINDOW decides it's done. This will help with multiple windows as well.
        while (!mWindow->ShouldClose()) {
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - lastFrameTime).count();
            lastFrameTime = now;

            mWindow->OnUpdate();

            mVulkanRenderer->BeginFrame();

            for (auto& layer : mLayersStack)
                layer->OnUpdate(dt);

            for (auto& layer : mLayersStack)
                layer->OnUIRender();

            mVulkanRenderer->OnImGui();
            ImGui::Render();
            mVulkanRenderer->Render();
        }
    }

}
