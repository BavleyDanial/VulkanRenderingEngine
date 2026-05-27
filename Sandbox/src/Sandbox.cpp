#include <EntryPoint.h>
#include <Engine.h>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace VKRE;

// NOTE: THIS IS JUST FOR ILLUSTRATION OF COMPUTE PASSES, WILL BE REMOVED
struct GradientParams {
    glm::vec4 colorA;
    glm::vec4 colorB;
};

/*
 * class FutureLayer : public Layer {
    FutureLayer()
        :Layer("Future Layer") {}

    virtual void OnAttach() {
        mScene = std::make_unique<Scene>();
        
        mTriangleMesh = AssetManager::LoadMesh("res/models/triangle.obj");
        mTriangleEntity = mScene->AddEntity("Moving Triangle");
        mTriangleEntity.Add<StaticMeshComponent>({ mTriangleMesh.Get(); });
    }

    virtual void OnDetach() {}

    virtual void OnUpdate(float dt) {
        mScene->OnUpdate(dt);
        Renderer::SubmitScene(mScene.get());
    }

    virtual void OnUIRender() {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::PopStyleVar();

        if (ImGui::Begin("Entity Inspector")) {
            TransformComponent& transform = mTriangleEntity.GetMutable<TransformComponent>();
            ImGui::Text("Entity: %s", mTriangleEntity.GetName());
            ImGui::DragFloat3("Position", glm::value_ptr(transform.Position));
            ImGui::DragFloat3("Rotation", glm::value_ptr(transform.Rotation), 0.1f);
            ImGui::DragFloat3("Scale", glm::value_ptr(transform.Scale), 0.1f);
        }
        ImGui::End();
    }

private:
    std::unique_ptr<Scene> mScene;
    Entity mTriangleEntity;
    ResourceRef<MeshTag> mTriangleMesh;

 * }
 */

class ExampleLayer : public Layer {
public:
    ExampleLayer()
        :Layer("Example Layer") {}

    virtual void OnAttach() {
        mScene = std::make_unique<Scene>();

        // NOTE: This is temporary, it will be moved out
        std::vector<Vertex> vertices = {
            { glm::vec3(-0.5f, -0.5f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec4(1.0f) },
            { glm::vec3( 0.5f, -0.5f, 0.0f), 1.0f, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec4(1.0f) },
            { glm::vec3( 0.0f,  0.5f, 0.0f), 0.5f, glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, glm::vec4(1.0f) },
        };

        std::vector<uint32_t> indices = { 0, 1, 2 };

        VKRE::MeshDesc desc{};
        desc.DebugName = "Triangle";
        desc.Vertices = vertices;
        desc.Indices = indices;

        // TODO: To be managed by an Asset System
        mMesh = Renderer::LoadMesh(std::move(desc));
        Renderer::UploadMesh(mMesh, vertices, indices);

        Entity Triangle = mScene->AddEntity("Triangle");

        MeshHotData* hot = Renderer::GetMeshHot(mMesh.Get());
        GPUBufferHotData* vertexHot = Renderer::GetGPUBufferHot(hot->VertexBuffer);

        mPushConstants.vertexBufferAddress = vertexHot->DeviceAddress;

        mMeshPass = Renderer::AddDrawPass({
            .shaderPath = "res/shaders/mesh.glsl",
            .debugName = "mesh",
            .pushConstantRanges = { { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants) } },
            .colorAttachmentFormats = { VK_FORMAT_R16G16B16A16_SFLOAT },
            .mesh = mMesh.Get()
        });

        mGradientPass = Renderer::AddComputePass({
            .shaderPath = "res/shaders/gradient.glsl",
            .debugName = "gradient",
            .pushConstantRanges = { { VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GradientParams) } }
        });

        mSkyPass = Renderer::AddComputePass({
            .shaderPath = "res/shaders/gradient.glsl",
            .debugName = "sky",
            .pushConstantRanges = { { VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GradientParams) } }
        });

        mGradientParams = { .colorA = {0, 0, 0, 1}, .colorB = {0, 0, 1, 1} };
        mSkyParams = { .colorA = {0.1f, 0.2f, 0.4f, 1}, .colorB = {0, 0.1f, .2f, 1} };

        Renderer::SetDrawPassData(mMeshPass, &mPushConstants, sizeof(mPushConstants));
        Renderer::SetComputePassData(mGradientPass, &mGradientParams, sizeof(mGradientParams));
        Renderer::SetComputePassData(mSkyPass, &mSkyParams, sizeof(mSkyParams));
    }

    virtual void OnDetach() {}
    virtual void OnUpdate(float dt) {}

    virtual void OnUIRender() {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::PopStyleVar();

        if (ImGui::Begin("GradientEffect")) {
            ImGui::SliderInt("Pass", &index, 0, 2);

            if (index == 0) {
                Renderer::ActivateDrawPass(mMeshPass);
                Renderer::DeActivateComputePass(mGradientPass);
                Renderer::DeActivateComputePass(mSkyPass);

            } else if (index == 1) {
                Renderer::ActivateComputePass(mGradientPass);
                Renderer::DeActivateComputePass(mSkyPass);

                if (ImGui::SliderFloat4("Color A", glm::value_ptr(mGradientParams.colorA), 0.0f, 1.0f) ||
                        ImGui::SliderFloat4("Color B", glm::value_ptr(mGradientParams.colorB), 0.0f, 1.0f)) {
                    Renderer::SetComputePassData(mGradientPass, &mGradientParams, sizeof(mGradientParams));
                }

            } else if (index == 2) {
                Renderer::ActivateComputePass(mSkyPass);
                Renderer::DeActivateComputePass(mGradientPass);

                if (ImGui::SliderFloat4("Color A", glm::value_ptr(mSkyParams.colorA), 0.0f, 1.0f) ||
                        ImGui::SliderFloat4("Color B", glm::value_ptr(mSkyParams.colorB), 0.0f, 1.0f)) {
                    Renderer::SetComputePassData(mSkyPass, &mSkyParams, sizeof(mSkyParams));
                }
            }

        }
        ImGui::End();
    }

private:
    // This will 100% stay here
    std::unique_ptr<Scene> mScene;

private:
    struct MeshPushConstants {
        uint64_t vertexBufferAddress = 0;
    };
    ResourceRef<VKRE::MeshTag> mMesh;
    DrawPassHandle mMeshPass = VKRE::INVALID_DRAW_PASS;
    MeshPushConstants mPushConstants;

    ComputePassHandle mGradientPass = VKRE::INVALID_COMPUTE_PASS;
    ComputePassHandle mSkyPass = VKRE::INVALID_COMPUTE_PASS;
    GradientParams mGradientParams{};
    GradientParams mSkyParams{};

    int index = 0;

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

