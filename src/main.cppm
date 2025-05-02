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
        for (const auto& info: g_LastProfileInfos) {
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

void ConvertMouseToWorldPos(float mouseX, float mouseY, const glm::mat4 &viewMatrix,
                            const glm::mat4 &projectionMatrix, glm::vec3 &worldPos) {
    auto size = Application::Get().GetWindow().GetSize();
    float width = static_cast<float>(size.first);
    float height = static_cast<float>(size.second);
    float ndcX = (2.0f * mouseX) / width - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / height;

    glm::vec4 ndcNear = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 ndcFar = glm::vec4(ndcX, ndcY, 1.0f, 1.0f);

    glm::mat4 invVP = glm::inverse(projectionMatrix * viewMatrix);
    glm::vec4 worldNear = invVP * ndcNear;
    glm::vec4 worldFar = invVP * ndcFar;

    if (worldNear.w != 0.0f) worldNear /= worldNear.w;
    if (worldFar.w != 0.0f) worldFar /= worldFar.w;

    auto rayOrigin = glm::vec3(worldNear);
    glm::vec3 rayDirection = glm::normalize(glm::vec3(worldFar) - rayOrigin);

    if (rayDirection.z != 0.0f) {
        float t = -rayOrigin.z / rayDirection.z;
        worldPos = rayOrigin + t * rayDirection;
    } else {
        worldPos = glm::vec3(rayOrigin.x, rayOrigin.y, 0.0f);
    }
}

class RendererLayer : public Layer {
    glm::mat4 viewMatrix{};
    glm::mat4 projectionMatrix{};

    virtual void OnUpdate(float) override {
        EZ_PROFILE_FUNCTION();
        auto size = Application::Get().GetWindow().GetSize();
        float aspectRatio = static_cast<float>(size.first) / static_cast<float>(size.second);
        auto camera = OrthographicCamera(-aspectRatio, aspectRatio, -1.0f, 1.0f);
        static float rotation = 0.0f;
        camera.SetRotation(rotation);
        rotation += 0.5f;
        Renderer2D::BeginScene(camera);

        viewMatrix = camera.GetViewMatrix();
        projectionMatrix = camera.GetProjectionMatrix();

        static float circleRotation = 0.0f;

        Renderer2D::DrawRect(glm::vec3{}, glm::vec2(1.f, .5f), glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));
        Renderer2D::DrawCircle(glm::rotate(glm::mat4(1.0f), glm::radians(circleRotation),
                                           glm::vec3(0.0f, 0.0f, 1.0f))
                               * glm::translate(
                                   glm::mat4{1.0f}, glm::vec3(0.5f, 0.5f, 0.0f)
                               ) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, 0.0f)),
                               glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));

        circleRotation += 0.7f;

        Renderer2D::EndScene();
    }

    virtual void OnEvent(Event &event) override {
        EZ_PROFILE_FUNCTION();
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent &e) {
            float mouseX = Input::GetMouseX();
            float mouseY = Input::GetMouseY();

            glm::vec3 worldPos;
            ConvertMouseToWorldPos(mouseX, mouseY, viewMatrix, projectionMatrix, worldPos);

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
