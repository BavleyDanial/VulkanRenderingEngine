#pragma once

#include <Core/Events/EventDispatcher.h>

#include <Window/GlfwWindow.h>
#include <Window/GlfwInput.h>

#include <Core/Layer.h>
#include <Core/LayerStack.h>

#include <ResourceManager/ResourceManager.h>

#include <Vulkan/VulkanContext.h>
#include <Vulkan/VulkanRenderer.h>

#include <memory>

namespace VKRE {

    class Application {
    public:
        Application();
        virtual ~Application();

        static Application& GetInstance() { return *mInstance; }
        virtual void Run();

        void PushLayer(std::unique_ptr<Layer> layer) { mLayersStack.PushLayer(std::move(layer)); }
        void PushOverlay(std::unique_ptr<Layer> overlay) { mLayersStack.PushOverlay(std::move(overlay)); }
        void PopLayer(std::unique_ptr<Layer> layer) { mLayersStack.PopLayer(std::move(layer)); }
        void PopOverlay(std::unique_ptr<Layer> overlay) { mLayersStack.PopOverlay(std::move(overlay)); }

        Window& GetWindow() { return *mWindow; }
        EventDispatcher& GetEventDispatcher() { return mEventDispatcher; }

    private:
        static inline Application* mInstance = nullptr;

        // Core
        EventDispatcher mEventDispatcher;
        ResourceManager mResourceManager;
        LayersStack mLayersStack;

        std::unique_ptr<Window> mWindow; // TODO: Make multiple windows possible (through an array with window ids? but then we need to make sure that each context is tied to the correct id? idk... For now this is fine especially when we add ImGui's multiviewport)

        // Vulkan
        std::unique_ptr<VulkanContext> mVulkanContext;
        std::unique_ptr<VulkanRenderer> mVulkanRenderer;
    };

    Application* CreateApplication();

}
