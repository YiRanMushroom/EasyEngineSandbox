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
import Easy.Platform.Impl.OpenGL.Renderer.Buffer;
import Easy.Platform.Impl.OpenGL.Renderer.VertexArray;
import Easy.Platform.Impl.OpenGL.Renderer.Shader;

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

class ImBackGroundLayer : public ImGuiDockerLayer {
    bool m_EnableDockerSpace = false;

    virtual void OnDockerRenderAdditional() override {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open Project...", "Ctrl+O")) {}
                // OpenProject();

                ImGui::Separator();

                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
                // NewScene();

                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
                // SaveScene();

                if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {}
                // SaveSceneAs();

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                    Application::Get().Close();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Script")) {
                if (ImGui::MenuItem("Reload assembly", "Ctrl+R")) {}
                // ScriptEngine::ReloadAssembly();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window")) {
                ImGui::MenuItem("Console", nullptr, &showDebugWindow);
                ImGui::MenuItem("Demo", nullptr, &showDemoWindow);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::MenuItem("Toggle Docker Space", nullptr)) {
                    m_EnableDockerSpace = !m_EnableDockerSpace;
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    bool m_NeedResize = false;

    virtual void OnImGuiRender() override {
        ImGuiDockerLayer::OnImGuiRender();

        if (m_EnableDockerSpace) {
            ImGui::Begin("Big Docker Space", &m_EnableDockerSpace);
            if (m_NeedResize) {
                ImGui::SetWindowSize(ImVec2(800, 600));
                m_NeedResize = false;
            }
            ImGui::Text("This is a big docker space.");
            ImGui::Text("You can drag and drop other docker windows here.");
            ImGui::End();
        } else {
            m_NeedResize = false;
        }
    }
};


class RendererLayer : public Layer {
    static inline const char *vertexShaderSource =
            "#version 450\n"
            "layout (location = 0) in vec3 aPos;\n"
            "void main() {\n"
            "    gl_Position = vec4(aPos, 1.0);\n"
            "}\0";

    static inline const char *fragmentShaderSource =
            "#version 450\n"
            "layout(location = 0) out vec4 FragColor;\n"
            "void main() {\n"
            "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
            "}\0";

    unsigned int shaderProgram;
    Arc<VertexArray> vertexArray;
    Arc<Shader> shader;

    virtual void OnAttach() override {
        static float vertices[] = {
            -0.5, 0.5, 0.0f, // top left
            0.5, 0.5, 0.0f, // top right
            -0.5, -0.5, 0.0f, // bottom left
            0.5, -0.5, 0.0f // bottom right
        };

        static uint32_t indices[] = {
            0, 1, 2,
            1, 2, 3
        };



        vertexArray = MakeArc<OpenGLVertexArray>();
        auto vertexBuffer = MakeArc<OpenGLVertexBuffer>(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({{ShaderDataType::Float3, "aPos"}});
        vertexArray->AddVertexBuffer(std::move(vertexBuffer));
        vertexArray->SetIndexBuffer(MakeArc<OpenGLIndexBuffer>(indices, sizeof(indices)));

        shader = MakeArc<OpenGLShader>("Triangle", vertexShaderSource, fragmentShaderSource);
    }

    virtual void OnUpdate(float) override {
        shader->Bind();
        vertexArray->Bind();
        glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
    }

    virtual void OnEvent(Event &event) override {
        std::cout << "RendererLayer::OnEvent: " << event.ToString() << std::endl;
    }
};

int main() {
    auto app =
            ApplicationBuilder::Start()
            .Window<OpenGLWindow>()
            .WindowWidth(1920)
            .WindowHeight(1080)
            .ImGuiLayer<OpenGLImGuiLayer>()
            .Build();

    app->PushLayer(MakeArc<BackGroundLayer>());
    app->PushLayer(MakeArc<RendererLayer>());
    app->PushLayer(MakeArc<ImBackGroundLayer>());
    app->PushLayer(MakeArc<SimpleImGuiLayer>());

    app->Run();
    return 0;
}
