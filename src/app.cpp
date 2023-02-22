#include "app.h"
#include "code_editor.h"
#include "scheme_utils.h"

#include <imgui.h>

using namespace flow;

struct Node {
    int id;
    float value;
};

#include <imnodes.h>
static void imnodes() {
    static auto node = Node{.id = 8, .value = 12};
    static auto ctx = ImNodes::EditorContextCreate();

    ImGui::Begin("Node Editor");
    ImNodes::EditorContextSet(ctx);
    ImNodes::BeginNodeEditor();

    ImNodes::BeginNode(node.id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("node");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(node.id << 8);
        ImGui::TextUnformatted("input");
        ImNodes::EndInputAttribute();
        ImGui::SameLine();

        ImNodes::BeginOutputAttribute(node.id << 24);
        const float text_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(120.f + ImGui::CalcTextSize("value").x - text_width);
        ImGui::TextUnformatted("output");
        ImNodes::EndOutputAttribute();

        ImNodes::BeginStaticAttribute(node.id << 16);
        ImGui::PushItemWidth(120.0f);
        ImGui::DragFloat("value", &node.value, 0.01f);
        ImGui::PopItemWidth();
        ImNodes::EndStaticAttribute();

        ImNodes::EndNode();

    ImNodes::MiniMap();
    ImNodes::EndNodeEditor();
    ImGui::End();
}

application::application(int, char**)
{
    scheme_init();
    scheme_add_resource_path("lib/s7");
}

void application::render()
{
    ImGui::Begin("Flow");
    ImGui::Text("TODO");
    ImGui::End();

    static auto old_editor = code_editor();
    static auto new_editor = editor();

    new_editor();

    ImGui::Begin("Scheme");
    old_editor.render();
    ImGui::End();

    imnodes();
}

void application::close()
{}