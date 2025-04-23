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
            "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "void main() {\n"
            "    gl_Position = vec4(aPos, 1.0);\n"
            "}\0";

    static inline const char *fragmentShaderSource =
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "void main() {\n"
            "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
            "}\0";

    static inline unsigned int shaderProgram, VAO, VBO;

    virtual void OnAttach() override {
        static float vertices[] = {
            -0.5f, -0.5f, 0.0f, // 左下
            0.5f, -0.5f, 0.0f, // 右下
            0.0f, 0.5f, 0.0f // 顶部
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    virtual void OnUpdate(float) override {
        // Draw a triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    virtual void OnEvent(Event &event) override {
        std::cout << "RendererLayer::OnEvent: " << event.ToString() << std::endl;
    }
};

int main() {
    auto app =
            ApplicationBuilder::Start()
            .Window<OpenGLWindow>()
            .ImGuiLayer<OpenGLImGuiLayer>()
            .Build();

    app->PushLayer(MakeArc<BackGroundLayer>());
    app->PushLayer(MakeArc<RendererLayer>());
    app->PushLayer(MakeArc<ImBackGroundLayer>());
    app->PushLayer(MakeArc<SimpleImGuiLayer>());

    app->Run();
    return 0;
}
