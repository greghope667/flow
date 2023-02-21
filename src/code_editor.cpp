#include "code_editor.h"

#include <chrono>
#include <iostream>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <s7.h>
#include "scheme.h"


using namespace flow;

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