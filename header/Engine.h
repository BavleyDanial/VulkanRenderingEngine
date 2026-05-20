#pragma once

#include <Window/GlfwWindow.h>
#include <ResourceManager/ResourceManager.h>

#include <Vulkan/VulkanContext.h>
#include <Vulkan/VulkanRenderer.h>

#include <memory>


// NOTE: THIS IS JUST FOR ILLUSTRATION OF COMPUTE PASSES, WILL BE REMOVED
struct GradientParams {
    glm::vec4 colorA;
    glm::vec4 colorB;
};

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

    // Core
    std::shared_ptr<VKRE::Window> mWindow; // TODO: Make multiple windows possible (through an array with window ids? but then we need to make sure that each context is tied to the correct id? idk... For now this is fine especially when we add ImGui's multiviewport)
    std::unique_ptr<VKRE::ResourceManager> mResourceManager;

    // Vulkan
    std::unique_ptr<VKRE::VulkanContext> mVulkanContext;
    std::unique_ptr<VKRE::VulkanRenderer> mVulkanRenderer;

    VKRE::DrawPassHandle mTrianglePass = VKRE::INVALID_DRAW_PASS;

    VKRE::ComputePassHandle mGradientPass = VKRE::INVALID_COMPUTE_PASS;
    VKRE::ComputePassHandle mSkyPass = VKRE::INVALID_COMPUTE_PASS;
    GradientParams mGradientParams{};
    GradientParams mSkyParams{};
};
