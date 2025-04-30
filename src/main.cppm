module;
#include "OpenGL.hpp"

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
// import Easy.Platform.Impl.OpenGL.Renderer.Buffer;
// import Easy.Platform.Impl.OpenGL.Renderer.VertexArray;
// import Easy.Platform.Impl.OpenGL.Renderer.Shader;

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
        glClearColor(0.6, 0.6, 0.6, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
};

class RendererLayer : public Layer {
    virtual void OnUpdate(float) override {
        auto size = Application::Get().GetWindow().GetSize();
        float aspectRatio = static_cast<float>(size.first) / static_cast<float>(size.second);
        auto camera = OrthographicCamera(-aspectRatio * 5, aspectRatio * 5, -5.0f, 5.0f);
        static float rotation = 0.0f;
        camera.SetRotation(rotation);
        rotation += 0.5f;
        Renderer2D::BeginScene(camera);

        static float circleRotation = 0.0f;

        Renderer2D::DrawRect(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.5f, 0.5f), glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));
        Renderer2D::DrawCircle(glm::rotate(glm::mat4(1.0f), glm::radians(circleRotation),
                                           glm::vec3(0.0f, 0.0f, 1.0f))
                               * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, 0.0f))
                               * glm::translate(
                                   glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f)),
                               glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));
        circleRotation += 0.7f;

        Renderer2D::EndScene();
    }

    virtual void OnEvent(Event &event) override {}
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
