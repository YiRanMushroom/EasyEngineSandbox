module;
#include "OpenGL.hpp"
#include "Core/MacroUtils.hpp"
export module main;

import Easy;

using namespace Easy;

static bool showDebugWindow = true;
static bool showDemoWindow = true;

class SimpleImGuiLayer : public Layer {
    float m_DeltaTime = 0.0f;

    virtual void OnUpdate(float deltaTime) override {
        EZ_PROFILE_SCOPE();
        m_DeltaTime = deltaTime;
    }

    virtual void OnImGuiRender() override {
        EZ_PROFILE_FUNCTION();

        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        if (showDebugWindow) {
            ImGui::ShowDebugLogWindow(&showDebugWindow);
        }

        ImGui::Begin("Display Deltatime");
        ImGui::Text("DeltaTime: %.3f", m_DeltaTime);
        ImGui::Text("FPS: %.1f", 1.0f / m_DeltaTime);
        ImGui::End();

        ImGui::Begin("Profile");
        for (const auto &info: g_LastProfileInfos) {
            char buffer[1024];
            info.writeTo(buffer);
            ImGui::Text("%s", buffer);
        }
        ImGui::End();
    }
};

class BackGroundLayer : public Layer {
    virtual void OnUpdate(float) override {
        EZ_PROFILE_FUNCTION();
        RenderCommand::SetClearColor({0.6f, 0.6f, 0.6f, 1.0f});
        RenderCommand::Clear();
    }
};

class RendererLayer : public Layer {
    glm::mat4 viewMatrix{};
    glm::mat4 projectionMatrix{};

    virtual void OnUpdate(float) override {
        EZ_PROFILE_FUNCTION();
        auto size = Application::Get().GetWindow().GetSize();
        float aspectRatio = static_cast<float>(size.first) / static_cast<float>(size.second);
        auto camera = OrthographicCamera(-aspectRatio * 5, aspectRatio * 5, -1.0f * 5, 1.0f * 5);
        static float rotation = 0.0f;
        camera.SetRotation(rotation);
        rotation += 0.5f;
        Renderer2D::BeginScene(camera);

        viewMatrix = camera.GetViewMatrix();
        projectionMatrix = camera.GetProjectionMatrix();

        static float circleRotation = 0.0f;

        for (size_t i = 0; i < 10000; i++) {
            Renderer2D::DrawRect(Easy::MakeTransform<Easy::Scale>(1.5, 0.5)
                                 .Then<Rotate>(i * 1.f, Rotate::Axis::Z)
                                 .Then<Translate>(i * .001f, i * .001f)
                                 .GetTransform(), glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));
        }

        Renderer2D::DrawCircle(Easy::MakeTransform<Easy::Scale>(1.5, 0.5)
                               // .Then<Translate>(0.5, 0.5)
                               .Then<Rotate>(circleRotation, Rotate::Axis::Z)
                               .GetTransform(), glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));

        // circleRotation += 0.7f;

        Renderer2D::EndScene();
    }

    virtual void OnEvent(Event &event) override {
        EZ_PROFILE_FUNCTION();
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent &e) {
            float mouseX = Input::GetMouseX();
            float mouseY = Input::GetMouseY();

            glm::vec3 worldPos;

            auto [width, height] = Application::Get().GetWindow().GetSize();

            Easy::ConvertMouseToWorldPos(width, height, mouseX, mouseY, viewMatrix, projectionMatrix, worldPos);

            EZ_INFO("Mouse Pressed at screen: ({0}, {1})", mouseX, mouseY);
            EZ_INFO("World position: ({0}, {1}, {2})", worldPos.x, worldPos.y, worldPos.z);
            return true;
        });
    }
};

int main() {
    auto app =
            ApplicationBuilder::Start()
            .Window<OpenGLWindow>()
            .WindowWidth(960)
            .WindowHeight(640)
            .ImGuiLayer<OpenGLImGuiLayer>()
            .Build();

    app->PushLayer(MakeArc<BackGroundLayer>());
    app->PushLayer(MakeArc<RendererLayer>());
    app->PushLayer(MakeArc<SimpleImGuiLayer>());

    app->Run();
    return 0;
}
