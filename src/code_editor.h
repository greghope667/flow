#pragma once

#include <string>

struct s7_scheme;

namespace flow {

struct code_editor {

    code_editor() : code_(), result_() {}

    bool auto_exec_;
    std::string code_;
    std::string result_;

    void render();
private:
    void exec();
};
}