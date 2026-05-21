#include <EntryPoint.h>
#include <Engine.h>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// NOTE: THIS IS JUST FOR ILLUSTRATION OF COMPUTE PASSES, WILL BE REMOVED
struct GradientParams {
    glm::vec4 colorA;
    glm::vec4 colorB;
};

class ExampleLayer : public VKRE::Layer {
public:
    ExampleLayer()
        :Layer("Example Layer") {}

    virtual void OnAttach() {
        // NOTE: This is temporary, it will be moved out
        mTrianglePass = VKRE::Renderer::AddDrawPass({
            .shaderPath = "res/shaders/colored_triangle.glsl",
            .debugName = "triangle",
            .colorAttachmentFormats = { VK_FORMAT_R16G16B16A16_SFLOAT },
            .vertexCount = 3,
        });

        mGradientPass = VKRE::Renderer::AddComputePass({
            .shaderPath = "res/shaders/gradient.glsl",
            .debugName = "gradient",
            .pushConstantRanges = { { VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GradientParams) } }
        });

        mSkyPass = VKRE::Renderer::AddComputePass({
            .shaderPath = "res/shaders/gradient.glsl",
            .debugName = "sky",
            .pushConstantRanges = { { VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GradientParams) } }
        });

        mGradientParams = { .colorA = {0, 0, 0, 1}, .colorB = {0, 0, 1, 1} };
        mSkyParams = { .colorA = {0.1f, 0.2f, 0.4f, 1}, .colorB = {0, 0.1f, .2f, 1} };

        VKRE::Renderer::SetComputePassData(mGradientPass, &mGradientParams, sizeof(mGradientParams));
        VKRE::Renderer::SetComputePassData(mSkyPass, &mSkyParams, sizeof(mSkyParams));
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
                VKRE::Renderer::ActivateDrawPass(mTrianglePass);
                VKRE::Renderer::DeActivateComputePass(mGradientPass);
                VKRE::Renderer::DeActivateComputePass(mSkyPass);

            } else if (index == 1) {
                VKRE::Renderer::ActivateComputePass(mGradientPass);
                VKRE::Renderer::DeActivateComputePass(mSkyPass);

                if (ImGui::SliderFloat4("Color A", glm::value_ptr(mGradientParams.colorA), 0.0f, 1.0f) ||
                        ImGui::SliderFloat4("Color B", glm::value_ptr(mGradientParams.colorB), 0.0f, 1.0f)) {
                    VKRE::Renderer::SetComputePassData(mGradientPass, &mGradientParams, sizeof(mGradientParams));
                }

            } else if (index == 2) {
                VKRE::Renderer::ActivateComputePass(mSkyPass);
                VKRE::Renderer::DeActivateComputePass(mGradientPass);

                if (ImGui::SliderFloat4("Color A", glm::value_ptr(mSkyParams.colorA), 0.0f, 1.0f) ||
                        ImGui::SliderFloat4("Color B", glm::value_ptr(mSkyParams.colorB), 0.0f, 1.0f)) {
                    VKRE::Renderer::SetComputePassData(mSkyPass, &mSkyParams, sizeof(mSkyParams));
                }
            }

        }
        ImGui::End();
    }

private:
    VKRE::DrawPassHandle mTrianglePass = VKRE::INVALID_DRAW_PASS;

    VKRE::ComputePassHandle mGradientPass = VKRE::INVALID_COMPUTE_PASS;
    VKRE::ComputePassHandle mSkyPass = VKRE::INVALID_COMPUTE_PASS;
    GradientParams mGradientParams{};
    GradientParams mSkyParams{};

    int index = 0;

};

class Sandbox : public VKRE::Application {
public:
    Sandbox() {
        PushLayer(std::make_unique<ExampleLayer>());
    }

};

VKRE::Application* VKRE::CreateApplication() {
    return new Sandbox();
}

