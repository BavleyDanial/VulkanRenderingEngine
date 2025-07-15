#include "GlfwWindow.h"
#include <Engine.h>

#include <cassert>
#include <memory>

Engine::Engine() {
    if (mInstance) {
        assert("Engine has already been initialised!");
    }

    mInstance = this;
    mWindow = std::make_unique<VKRE::Window>(VKRE::WindowSpecs{});
    mVulkanContext = std::make_unique<VKRE::VulkanContext>();
}

void Engine::Run() {
    while (!mWindow->ShouldClose()) {
        mWindow->OnUpdate();
    }
}
