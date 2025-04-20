module;
#include "imgui.h"

export module main;

import Easy.Core.Basic;
import Easy.Core.Application;
import Easy.Core.Layer;
import Easy.Events.Event;
import Easy.ImGui.ImGuiLayer;
import Easy.Platform.Impl.OpenGLWindow;
import Easy.Platform.Impl.OpenGLImGuiLayer;

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

class BackGroundLayer : public ImGuiDockerLayer {
    virtual void OnUpdate(float) override {
        // temporary
        glClearColor(0.6, 0.6, 0.6, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

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

            ImGui::EndMenuBar();
        }
    }
};

class RendererLayer : public Layer {
    virtual void OnEvent(Event &event) override {
        std::cout << "RendererLayer::OnEvent: " << event.ToString() << std::endl;
    }
};

int main() {
    auto app = MakeArc<Application>(ApplicationSpecification::Make<OpenGLWindow, OpenGLImGuiLayer>("EasyEngineCore"));
    app->PushLayer(MakeArc<BackGroundLayer>());
    app->PushLayer(MakeArc<SimpleImGuiLayer>());
    app->PushLayer(MakeArc<RendererLayer>());
    app->Run();
    return 0;
}
