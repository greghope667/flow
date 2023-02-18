#pragma once

#include <string>

struct s7_scheme;

namespace flow {

struct code_editor {

    code_editor(s7_scheme* s7) : code_(), result_(), s7_(s7) {}

    bool auto_exec_;
    std::string code_;
    std::string result_;
    s7_scheme* s7_;

    void render();
private:
    void exec();
};
}