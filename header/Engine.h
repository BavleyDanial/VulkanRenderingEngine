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

    static const Engine& GetInstance() { return *mInstance; }
    const VKRE::Window* GetWindow() const { return mWindow.get(); }
    const VKRE::VulkanContext* GetRenderingContext() const { return mVulkanContext.get(); }

private:
    static inline Engine* mInstance = nullptr;

    std::shared_ptr<VKRE::Window> mWindow;
    std::shared_ptr<VKRE::VulkanContext> mVulkanContext;
    std::shared_ptr<VKRE::VulkanRenderer> mVulkanRenderer;
};
