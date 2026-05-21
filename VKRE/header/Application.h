#pragma once

#include <Window/GlfwWindow.h>
#include <Window/GlfwInput.h>

#include <Layer.h>
#include <LayerStack.h>

#include <ResourceManager/ResourceManager.h>

#include <Vulkan/VulkanContext.h>
#include <Vulkan/VulkanRenderer.h>

#include <memory>

namespace VKRE {

    class Application {
    public:
        Application();
        virtual ~Application();

        void PushLayer(std::unique_ptr<Layer> layer) { mLayersStack.PushLayer(std::move(layer)); }
        void PushOverlay(std::unique_ptr<Layer> overlay) { mLayersStack.PushOverlay(std::move(overlay)); }
        void PopLayer(std::unique_ptr<Layer> layer) { mLayersStack.PopLayer(std::move(layer)); }
        void PopOverlay(std::unique_ptr<Layer> overlay) { mLayersStack.PopOverlay(std::move(overlay)); }

        virtual void Run();
        static Application& GetInstance() { return *mInstance; }

    public:
        // TODO: Make this an event system... For now just a way to know if we're resizing the window is fine
        bool hasResized = false;

    private:
        static inline Application* mInstance = nullptr;

        // Core
        std::shared_ptr<VKRE::Window> mWindow; // TODO: Make multiple windows possible (through an array with window ids? but then we need to make sure that each context is tied to the correct id? idk... For now this is fine especially when we add ImGui's multiviewport)
        std::unique_ptr<VKRE::ResourceManager> mResourceManager;
        VKRE::LayersStack mLayersStack;

        // Vulkan
        std::unique_ptr<VKRE::VulkanContext> mVulkanContext;
        std::unique_ptr<VKRE::VulkanRenderer> mVulkanRenderer;
    };

    Application* CreateApplication();

}
