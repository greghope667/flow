#include "app.h"
#include "code_editor.h"
#include "flow.h"
#include "scheme_utils.h"

#include <imgui.h>

using namespace flow;

struct Node {
    int id;
    float value;
};

application::application(int, char**)
{
    scheme_init();
    scheme_add_resource_path("lib/s7");
    for (int i=0; i<10; i++)
        test_scene();
    test_undefined();
}

void application::render()
{
    static auto old_editor = code_editor();
    static auto new_editor = editor();

    new_editor();

    ImGui::Begin("Scheme");
    old_editor.render();
    ImGui::End();
}

void application::close()
{}