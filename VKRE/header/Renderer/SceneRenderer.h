#pragma once

#include <Scene/Scene.h>

#include <Renderer/Renderer.h>
#include <Assets/Texture.h>

namespace VKRE {

    class SceneRenderer {
    public:
        SceneRenderer();

        void SetCamera(Entity camera) { mCamera = camera; }
        Entity GetCamera() const { return mCamera; }

        void SetScene(const Scene* scene) { mScene = scene; }
        const Scene* GetScene() const { return mScene; }

        void SetSkybox(const TextureCubeAsset* cubemap) { mSkybox = cubemap; }
        const TextureCubeAsset* GetSkybox() const { return mSkybox; }

        void Render();
        uint32_t GetDrawCalls() const { return mDrawCalls; } // TODO: Make a statistics panel that can be turned on and off

    private:
        void RenderScene();
        void RenderSkybox();

    private:
        Entity mCamera;
        const Scene* mScene;
        const TextureCubeAsset* mSkybox;

        glm::mat4 mCachedViewMat;
        glm::mat4 mCachedProjMat;

        DrawPassHandle mDrawPass;
        DrawPassHandle mSkyboxLDRPass;
        DrawPassHandle mSkyboxHDRPass;
        uint32_t mDrawCalls;
    };

}
