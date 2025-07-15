#pragma once

#include <GlfwWindow.h>
#include <VulkanContext.h>

#include <memory>

class Engine {
    public:
        Engine();
        void Run();

        static const Engine* GetInstance() { return mInstance; }
        const VKRE::Window* GetWindow() const { return mWindow.get(); }
        const VKRE::VulkanContext* GetRenderingContext() const { return mVulkanContext.get(); }

    private:
        static inline Engine* mInstance = nullptr;

        std::unique_ptr<VKRE::Window> mWindow;
        std::unique_ptr<VKRE::VulkanContext> mVulkanContext;

};
