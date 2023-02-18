#include "app.h"
#include "code_editor.h"

#include <imgui.h>

using namespace flow;

struct Node {
    int id;
    float value;
};

#include <imnodes.h>
static void imnodes() {
    static auto _ = []{ ImNodes::CreateContext(); return true; }();
    static auto node = Node{.id = 8, .value = 12};

    ImGui::Begin("Node Editor");
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

#include <s7.h>
static const char *pp(s7_scheme *sc, s7_pointer obj) /* (pp obj) */
{
  return(s7_string(
          s7_eval_c_string_with_environment(sc,
            "(catch #t                         \
               (lambda ()                      \
                 (require write.scm)           \
                 (pp obj))                     \
               (lambda (type info)             \
                 (apply format #f info)))",
	   s7_inlet(sc, s7_list(sc, 1, s7_cons(sc, s7_make_symbol(sc, "obj"), obj))))));
}

#include <filesystem>
static bool add_resource_path(s7_scheme* sc, const char* path)
{
    auto curr = std::filesystem::current_path();
    auto full_path = curr;
    for (;;) {
        full_path = curr / path;
        if (std::filesystem::exists(full_path)) break;
        if (not curr.has_parent_path()) return false;
        curr = curr.parent_path();
    }
    s7_symbol_set_value(sc, s7_make_symbol(sc, "*load-path*"),
    s7_append(sc, s7_name_to_value(sc, "*load-path*"), s7_list(sc, 1, s7_make_string(sc, full_path.c_str()))));
    return true;
}

void application::render()
{
    ImGui::Begin("Flow");
    ImGui::Text("TODO");
    ImGui::End();

    static auto s7 = s7_init();
    static auto _ = add_resource_path(s7, "lib/s7");
    static auto editor = code_editor(s7);

    ImGui::Begin("Scheme");
    editor.render();
    ImGui::End();

    imnodes();
}