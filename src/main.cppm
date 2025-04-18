module;
#include "imgui.h"
#include <imgui_internal.h>

export module main;

import Easy.Core.Basic;
import Easy.Core.Application;
import Easy.Core.Layer;

using namespace Easy;

static bool showDebugWindow = true;
static bool showDemoWindow = true;

class SimpleImGuiLayer : public Layer {
    virtual void OnImGuiRender() override {
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);


        if (showDebugWindow) {
            ImGui::ShowDebugLogWindow(&showDebugWindow);
        }
    }
};

class BackGroundLayer : public Layer {
    virtual void OnUpdate(float) override {
        // temporary
        glClearColor(0.6, 0.6, 0.6, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    virtual void OnImGuiRender() override {
        // Note: Switch this to true to enable dockspace
        static bool dockspaceOpen = true;
        static bool opt_fullscreen_persistant = true;
        bool opt_fullscreen = opt_fullscreen_persistant;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO &io = ImGui::GetIO();
        ImGuiStyle &style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSizeX;

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

        ImGui::End();
    }
};

int main() {
    Arc<Application> app = Arc<Application>{new Application(ApplicationSpecification{"EasyEngineCore"})};
    app->PushLayer(Arc<Layer>{new BackGroundLayer()});
    app->PushLayer(Arc<Layer>{new SimpleImGuiLayer()});
    app->Run();
    return 0;
}
