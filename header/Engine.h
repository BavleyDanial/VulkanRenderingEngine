#pragma once

#include <Window/GlfwWindow.h>

#include <Vulkan/VulkanContext.h>
#include <Vulkan/VulkanRenderer.h>

#include <memory>

class Engine {
public:
    Engine();
    ~Engine();

    void Run();

    static Engine& GetInstance() { return *mInstance; }

public:
    // TODO: Make this an event system... For now just a way to know if we're resizing the window is fine
    bool hasResized = false;

private:
    static inline Engine* mInstance = nullptr;

    // TODO: Make multiple windows possible (through an array with window ids? but then we need to make sure that each context is tied to the correct id? idk... For now this is fine especially when we add ImGui's multiviewport)
    std::shared_ptr<VKRE::Window> mWindow;
    std::shared_ptr<VKRE::VulkanContext> mVulkanContext;
    std::shared_ptr<VKRE::VulkanRenderer> mVulkanRenderer;
};
