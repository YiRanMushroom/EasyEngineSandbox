module;
#include "OpenGL.hpp"
#include "Core/MacroUtils.hpp"
export module main;

import Easy.Core.Basic;
import Easy.Core.Application;
import Easy.Core.Layer;
import Easy.Events.Event;
import Easy.ImGui.ImGuiLayer;
import Easy.Platform.Impl.OpenGL.Window;
import Easy.Platform.Impl.OpenGL.ImGuiLayer;
import Easy.ImGui.ImGui;
import Easy.Renderer.Buffer;
import Easy.Renderer.VertexArray;
import Easy.Renderer.Shader;
import Easy.Renderer.Renderer2D;
import easy.vendor.glm;
import Easy.Core.Input;
import Easy.Renderer.OrthographicCamera;
import Easy.Events.MouseEvents;
import Easy.Renderer.RenderCommand;

import Easy.Renderer.ShaderSources;

using namespace Easy;

static bool showDebugWindow = true;
static bool showDemoWindow = true;

class SimpleImGuiLayer : public Layer {
    float m_DeltaTime = 0.0f;

    virtual void OnUpdate(float deltaTime) override {
        m_DeltaTime = deltaTime;
    }

    virtual void OnImGuiRender() override {
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        if (showDebugWindow) {
            ImGui::ShowDebugLogWindow(&showDebugWindow);
        }

        ImGui::Begin("Display Deltatime");
        ImGui::Text("DeltaTime: %.3f", m_DeltaTime);
        ImGui::Text("FPS: %.1f", 1.0f / m_DeltaTime);
        ImGui::End();
    }
};

class BackGroundLayer : public Layer {
    virtual void OnUpdate(float) override {
        RenderCommand::SetClearColor({0.6f, 0.6f, 0.6f, 1.0f});
        RenderCommand::Clear();
        // glClearColor(0.6, 0.6, 0.6, 1.0);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
};

void ConvertMouseToWorldPos(float mouseX, float mouseY, const glm::mat4 &viewMatrix,
                            const glm::mat4 &projectionMatrix, glm::vec3 &worldPos) {
    // 获取窗口尺寸和NDC转换与原来相同
    auto size = Application::Get().GetWindow().GetSize();
    float width = static_cast<float>(size.first);
    float height = static_cast<float>(size.second);
    float ndcX = (2.0f * mouseX) / width - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / height;

    // 创建近平面和远平面上的点
    glm::vec4 ndcNear = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 ndcFar = glm::vec4(ndcX, ndcY, 1.0f, 1.0f);

    // 转换到世界空间
    glm::mat4 invVP = glm::inverse(projectionMatrix * viewMatrix);
    glm::vec4 worldNear = invVP * ndcNear;
    glm::vec4 worldFar = invVP * ndcFar;

    // 透视除法
    if (worldNear.w != 0.0f) worldNear /= worldNear.w;
    if (worldFar.w != 0.0f) worldFar /= worldFar.w;

    // 射线与z=0平面相交
    glm::vec3 rayOrigin = glm::vec3(worldNear);
    glm::vec3 rayDirection = glm::normalize(glm::vec3(worldFar) - rayOrigin);

    // 计算射线与z=0平面相交点
    if (rayDirection.z != 0.0f) {
        float t = -rayOrigin.z / rayDirection.z;
        worldPos = rayOrigin + t * rayDirection;
    } else {
        // 射线平行于z=0平面，无交点
        worldPos = glm::vec3(rayOrigin.x, rayOrigin.y, 0.0f);
    }
}

class RendererLayer : public Layer {
    glm::mat4 viewMatrix{};
    glm::mat4 projectionMatrix{};

    virtual void OnUpdate(float) override {
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

        Renderer2D::DrawRect(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.f, .5f), glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));
        Renderer2D::DrawCircle(glm::rotate(glm::mat4(1.0f), glm::radians(circleRotation),
                                           glm::vec3(0.0f, 0.0f, 1.0f))
                               * glm::translate(
                                   glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f)
                               ) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, 0.0f)),
                               glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));
        circleRotation += 0.7f;

        Renderer2D::EndScene();
    }

    virtual void OnEvent(Event &event) override {
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
    GLShaderSources::Init();
    auto app =
            ApplicationBuilder::Start()
            .Window<OpenGLWindow>()
            .WindowWidth(1920)
            .WindowHeight(1080)
            .ImGuiLayer<OpenGLImGuiLayer>()
            .Build();

    app->PushLayer(MakeArc<BackGroundLayer>());
    app->PushLayer(MakeArc<RendererLayer>());
    app->PushLayer(MakeArc<SimpleImGuiLayer>());

    app->Run();
    return 0;
}
