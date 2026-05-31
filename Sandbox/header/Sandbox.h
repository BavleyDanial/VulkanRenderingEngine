#pragma once

#include <Engine.h>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace VKRE;

class SandboxLayer : public Layer {
public:
    SandboxLayer()
        :Layer("Sandbox Layer") {}

    virtual void OnAttach();
    virtual void OnDetach();
    virtual void OnUpdate(float dt);
    virtual void OnUIRender();

private:
    std::unique_ptr<Scene> mScene;
    SceneRenderer mSceneRenderer;
    int32_t mFPS;

    Entity mCamera;
    Entity mTeapot;
    Entity mSponza;
    Entity mSun;

    float mSpeed = 40.0f;
    float mSensititvity = 60.0f;
    float mLastMouseX = 0;
    float mLastMouseY = 0;
    bool mJustLocked = false;
};
