#include "code_editor.h"

#include <chrono>
#include <iostream>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <s7.h>


static std::string
pretty(s7_scheme *sc, s7_pointer obj)
{
  return s7_string(
          s7_eval_c_string_with_environment(sc,
            "(catch #t                         \
               (lambda ()                      \
                 (require write.scm)           \
                 (pp obj))                     \
               (lambda (type info)             \
                 (apply format #f info)))",
	   s7_inlet(sc, s7_list(sc, 1, s7_cons(sc, s7_make_symbol(sc, "obj"), obj)))));
}

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

//using execution_clock = std::chrono::steady_clock;
//
//static execution_clock::time_point start_time;
//
//static void check_timeout(s7_scheme* s7, bool* all_done)
//{
//    auto elapsed = execution_clock::now() - start_time;
//    std::cout << elapsed << std::endl;
//    if (elapsed > std::chrono::milliseconds(1000))
//        *all_done = true;
//}

void code_editor::exec()
{
    auto env = s7_inlet(s7_, s7_nil(s7_));
//    start_time = execution_clock::now();
//    s7_set_begin_hook(s7_, check_timeout);
    auto result = s7_eval_c_string_with_environment(s7_, code_.c_str(), env);
    result_ = pretty(s7_, result);
    fflush(stdout);
}