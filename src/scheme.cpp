#include "scheme.h"

#include <s7.h>
#include <filesystem>

using namespace flow;

s7_scheme* flow::s7 = nullptr;

void flow::scheme_init()
{
    if (not s7)
        s7 = s7_init();
}

void flow::scheme_free()
{
    if (s7)
        s7_free(s7);
}

scheme_value::scheme_value(s7_pointer val) noexcept
{
    loc_ = s7_gc_protect(s7, val);
}

scheme_value::~scheme_value() noexcept
{
    if (ptr_)
        s7_gc_unprotect_at(s7, loc_);
    ptr_ = nullptr;
    loc_ = 0;
}

const char* scheme_value::pretty_print() const noexcept
{
    return ::flow::pretty_print(ptr_);
}

const char* flow::pretty_print(s7_pointer obj)
{
    const char* pp = 
            "(catch #t                         \
               (lambda ()                      \
                 (require write.scm)           \
                 (pp obj))                     \
               (lambda (type info)             \
                 (apply format #f info)))";
    auto env = s7_inlet(s7, s7_list(s7, 1, s7_cons(s7, s7_make_symbol(s7, "obj"), obj)));

    return s7_string(s7_eval_c_string_with_environment(s7, pp, env));
}

bool flow::add_resource_path(const char* path)
{
    auto curr = std::filesystem::current_path();
    auto full_path = curr;
    for (;;) {
        full_path = curr / path;
        if (std::filesystem::exists(full_path)) break;
        if (not curr.has_parent_path()) return false;
        curr = curr.parent_path();
    }

    s7_symbol_set_value(s7, s7_make_symbol(s7, "*load-path*"),
        s7_append(s7, 
            s7_name_to_value(s7, "*load-path*"), 
            s7_list(s7, 1, s7_make_string(s7, full_path.c_str()))
    ));

    return true;
}