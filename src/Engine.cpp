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
        mVulkanRenderer->Render();
    }
}
