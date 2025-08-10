#include <Engine.h>

#include <cassert>
#include <memory>

Engine::Engine() {
    if (mInstance) {
        assert("Engine has already been initialised!");
    }

    mInstance = this;
    
    mWindow = std::make_shared<VKRE::Window>(VKRE::WindowSpecs{});
    mVulkanContext = std::make_shared<VKRE::VulkanContext>(mWindow);
    mVulkanRenderer = std::make_shared<VKRE::VulkanRenderer>(mVulkanContext);
}

Engine::~Engine() {
    mVulkanRenderer.reset();
    mVulkanContext.reset();
    mWindow.reset();
}

void Engine::Run() {
    while (!mWindow->ShouldClose()) {
        mWindow->OnUpdate();
        mVulkanRenderer->Render();
    }
}
