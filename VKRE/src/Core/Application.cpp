#include <Core/Application.h>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ImGui/Backend/ImGuiGLFW.h>
#include <ImGui/Backend/ImGuiVulkan.h>

#include <Renderer/Renderer.h>

namespace VKRE {

    struct Position {
        float x, y, z;
    };

    Application::Application() {
        if (mInstance) {
            assert("Application has already been initialised!");
        }

        mInstance = this;

        mWindow = std::make_unique<Window>(WindowSpecs{ .resizable = true });
        mVulkanContext = std::make_unique<VulkanContext>(*mWindow);
        mVulkanRenderer = std::make_unique<VulkanRenderer>(*mVulkanContext, mResourceManager);
        Renderer::SetRenderer(mVulkanRenderer.get());
        Renderer::SetResourceManager(&mResourceManager);
    }

    Application::~Application() {
        mVulkanRenderer.reset();
        mVulkanContext.reset();
        mWindow.reset();
    }

    void Application::Run() {
        flecs::world ecs;
        auto e = ecs.entity("TestEntity").set<Position>({0.0f, 0.0f, 0.0f});
        const Position pos = e.get<Position>();
        std::println("Position is: x={}, y={}, z={}", pos.x, pos.y, pos.z);

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
