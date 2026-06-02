#pragma once

#include <Scene/Scene.h>

#include <Renderer/Renderer.h>

namespace VKRE {

    class SceneRenderer {
    public:
        SceneRenderer();

        void SetScene(const Scene* scene) { mScene = scene; }
        const Scene* GetScene() const { return mScene; }

        void SetCamera(Entity camera) { mCamera = camera; }
        Entity GetCamera() const { return mCamera; }

        void Render();
        uint32_t GetDrawCalls() const { return mDrawCalls; } // TODO: Make a statistics panel that can be turned on and off

    private:
        const Scene* mScene;
        Entity mCamera;

        DrawPassHandle mDrawPass;
        uint32_t mDrawCalls;
    };

}
