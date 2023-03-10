#include "code_editor.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <imgui.h>
#include <imnodes.h>
#include <misc/cpp/imgui_stdlib.h>
#include <s7.h>
#include "flow.h"
#include "scheme_utils.h"


using namespace flow;

static void render_editor_function(scene& s, function_id_t id)
{
    auto& func = s.get_function(id);
    ImNodes::BeginNode(int(id));

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("lambda");
    ImGui::SameLine(125);
    if (ImGui::Button(" > ", ImVec2(25,15))) {
        s.exec(func, true);
    }
    ImNodes::EndNodeTitleBar();

    ImGui::PushItemWidth(150.0f);
    for (auto input_id: func.inputs) {
        auto& input = s.get_port(input_id);
        assert(input.parent == id);
        assert(input.is_input);

        ImNodes::BeginInputAttribute(int(input_id));
        ImGui::InputText("", &input.name);
        ImNodes::EndInputAttribute();
    }

    for (auto output_id: func.outputs) {
        auto& output = s.get_port(output_id);
        assert(output.parent == id);
        assert(not output.is_input);

        ImNodes::BeginOutputAttribute(int(output_id));
        ImGui::InputText("", &output.name);
        ImNodes::EndInputAttribute();
    }

    ImGui::InputTextMultiline("", &func.code, ImVec2(0,50), ImGuiInputTextFlags_AllowTabInput);
    ImGui::Text("%.20s", func.result.c_str());

    ImGui::PopItemWidth();

    if (ImGui::Button("Add input")) {
        s.add_input(id);
    }

    ImGui::SameLine(75);

    if (ImGui::Button("Add output")) {
        s.add_output(id);
    }

    if (ImGui::Button("Run")) {
        s.exec(func, true);
    }

    ImNodes::EndNode();
}

static void render_editor_controls(scene& s)
{
    if (ImGui::Button("Add Function")) {
        auto added_function_id = s.add_function();
        auto [x,y] = ImGui::GetWindowSize();
        ImNodes::SetNodeEditorSpacePos(int(added_function_id), ImVec2(x/3,y/3));
    }

    ImGui::SameLine();

    if (ImGui::Button("Remove Selected") || ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        if (int length = ImNodes::NumSelectedLinks(); length > 0) {
            std::vector<int> to_remove(length);
            ImNodes::GetSelectedLinks(to_remove.data());
            for (auto i: to_remove) {
                s.remove_pipe(pipe_id_t(i));
            }
        }
        ImNodes::ClearLinkSelection();

        if (int length = ImNodes::NumSelectedNodes(); length > 0) {
            std::vector<int> to_remove(length);
            ImNodes::GetSelectedNodes(to_remove.data());
            for (auto i: to_remove) {
                s.remove_function(function_id_t(i));
            }
        }
        ImNodes::ClearNodeSelection();
    }

    if (ImGui::Button("Step")) {
        s.exec_step();
    }
}

renderer flow::editor()
{
    auto ctx = ImNodes::EditorContextCreate();
    return [s=scene(), ctx] () mutable {
        ImGui::Begin("A Node Editor");

        render_editor_controls(s);

        ImNodes::EditorContextSet(ctx);
        ImNodes::BeginNodeEditor();

        for (auto i: s.all_functions()) {
            render_editor_function(s, function_id_t(i));
        }

        for (auto i: s.all_pipes()) {
            auto& [data, src, dest] = s.get_pipe(pipe_id_t(i));

            if (data) ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(255, 40, 40, 255));
            ImNodes::Link(int(i), int(src), int(dest));
            if (data) ImNodes::PopColorStyle();
        }

        ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
        ImNodes::EndNodeEditor();

        int start, end;
        if (ImNodes::IsLinkCreated(&start, &end)) {
            s.add_pipe(port_id_t(start), port_id_t(end));
        }

        int hovered;
        if (ImNodes::IsLinkHovered(&hovered)) {
            auto& pipe = s.get_pipe(pipe_id_t(hovered));
            if (pipe.data) {
                ImGui::SetTooltip("%s\n", pipe.data.pretty_print());
            } else {
                ImGui::SetTooltip("[empty]");
            }
        }

        ImGui::End();
    };
}

void code_editor::render()
{
    if (ImGui::InputTextMultiline("SchemeText", &code_) && auto_exec_) {
        exec();
    }
    if (ImGui::Button("Run")) {
        exec();
    }
    ImGui::Checkbox("Auto Exec", &auto_exec_);
    ImGui::TextWrapped("%s", result_.c_str());
}

void code_editor::exec()
{
    auto pp = [&](auto ptr){return pretty_print(ptr);};
    auto env = s7_inlet(s7, s7_nil(s7));
    printf("Before: %s\n", pp(env));
    auto result = s7_eval_c_string_with_environment(s7, code_.c_str(), env);
    printf("After: %s\n", pp(env));
    printf("x = : %s\n", pp(s7_eval_c_string_with_environment(s7, "x", env)));
    result_ = pp(result);
    fflush(stdout);
}
