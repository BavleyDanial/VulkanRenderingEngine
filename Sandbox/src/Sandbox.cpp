#include <EntryPoint.h>
#include <Engine.h>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace VKRE;

class ExampleLayer : public Layer {
public:
    ExampleLayer()
        :Layer("Example Layer") {}

    virtual void OnAttach() {
        mScene = std::make_unique<Scene>();
        mCamera = mScene->AddCamera("camera");
        mSceneRenderer.SetScene(mScene.get());
        mSceneRenderer.SetCamera(mCamera);

        const MeshAsset* teapotMesh = AssetManager::LoadMesh("res/models/teapot.obj");
        mTeapot = mScene->AddEntity("Teapot");
        mTeapot.Add<StaticMeshComponent>({ teapotMesh });
        mTeapot.Add<TransformComponent>({ .Position = glm::vec3(0.0f, 0.0f, 50.0f), .Rotation = glm::vec3(-90.0f, 0.0f, 0.0f) });
    }

    virtual void OnDetach() {}
    virtual void OnUpdate(float dt) {
        TransformComponent& transform = mCamera.GetMutable<TransformComponent>();

        if (Input::KeyPressed(GLFW_KEY_F))
            Input::LockMouse();

        if (Input::KeyPressed(GLFW_KEY_ESCAPE))
            Input::UnLockMouse();

        if (Input::IsMouseLocked()) {
            auto [mouseX, mouseY] = Input::GetMousePosition();
            if (!mJustLocked) {
                float dx = (mouseX - mLastMouseX) * mSensititvity * dt;
                float dy = (mouseY - mLastMouseY) * mSensititvity * dt;

                transform.Rotation.y += dx;
                transform.Rotation.x += dy;

                transform.Rotation.x = glm::clamp(transform.Rotation.x, -89.0f, 89.0f);
            }

            mLastMouseX = mouseX;
            mLastMouseY = mouseY;
            mJustLocked = false;
        } else {
            mJustLocked = true;
        }

        glm::quat rotation = glm::quat(glm::radians(transform.Rotation));
        glm::vec3 forward = rotation * glm::vec3(0, 0, 1);
        glm::vec3 right = rotation * glm::vec3(1, 0, 0);
        glm::vec3 up = rotation * glm::vec3(0, 1, 0);

        if (Input::KeyHeld(GLFW_KEY_W)) {
            transform.Position += forward * mSpeed * dt;
        }

        if (Input::KeyHeld(GLFW_KEY_S)) {
            transform.Position -= forward * mSpeed * dt;
        }

        if (Input::KeyHeld(GLFW_KEY_A)) {
            transform.Position -= right * mSpeed * dt;
        }

        if (Input::KeyHeld(GLFW_KEY_D)) {
            transform.Position += right * mSpeed * dt;
        }

        if (Input::KeyHeld(GLFW_KEY_E)) {
            transform.Position.y += mSpeed * dt;
        }

        if (Input::KeyHeld(GLFW_KEY_Q)) {
            transform.Position.y -= mSpeed * dt;
        }

        mScene->OnUpdate(dt);
        mSceneRenderer.Render();
    }

    virtual void OnUIRender() {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::PopStyleVar();

        if (ImGui::Begin("CamSettings")) {
            ImGui::DragFloat("speed", &mSpeed, 0.1f, 0.0f);
            ImGui::DragFloat("sensitivity", &mSensititvity, 0.1f, 0.0f);
            ImGui::Separator();
        }
        ImGui::End();

        mScene->GetFlecsWorld().each([&](flecs::entity e, TransformComponent& transform) {
            if (ImGui::Begin(e.name())) {
                ImGui::PushID(static_cast<uint32_t>(e.id()));
                ImGui::DragFloat3("Position", glm::value_ptr(transform.Position), 0.1f);
                ImGui::DragFloat3("Rotation", glm::value_ptr(transform.Rotation), 0.1f);
                ImGui::DragFloat3("Scale", glm::value_ptr(transform.Scale), 0.1f);
                ImGui::PopID();
            }
            ImGui::End();
        });

    }

private:
    std::unique_ptr<Scene> mScene;
    SceneRenderer mSceneRenderer;

    Entity mCamera;
    Entity mTeapot;

    float mSpeed = 40.0f;
    float mSensititvity = 10.0f;
    float mLastMouseX = 0;
    float mLastMouseY = 0;
    bool mJustLocked = false;
};

class Sandbox : public Application {
public:
    Sandbox() {
        PushLayer(std::make_unique<ExampleLayer>());
    }

};

VKRE::Application* VKRE::CreateApplication() {
    return new Sandbox();
}

