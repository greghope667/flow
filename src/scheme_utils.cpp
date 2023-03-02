#include "scheme_utils.h"

#include <s7.h>
#include <filesystem>
#include <unordered_set>
#include <cstdlib>

// #define FLOW_GC_TRACE

using namespace flow;

s7_scheme* flow::s7 = nullptr;

static s7_pointer notify(s7_scheme*, s7_pointer val)
{
    std::string v = pretty_print(val);
    v = "notify-send \"" + v + "\"";
    system(v.c_str());
    return val;
}

void flow::scheme_init()
{
    if (not s7)
        s7 = s7_init();
    s7_define_function(s7, "notify", notify, 1, 0, false, "Help text");
}

void flow::scheme_free()
{
    if (s7)
        s7_free(s7);
}

void flow::scheme_abort_execution()
{
    if (s7)
        s7_quit(s7);
}

#ifdef FLOW_GC_TRACE
std::unordered_set<void*> gc_protected{};
#endif

scheme_value::scheme_value(s7_pointer val) noexcept
{
    ptr_ = val;
    loc_ = 0;

    if (ptr_) {
        loc_ = s7_gc_protect(s7, val);
#ifdef FLOW_GC_TRACE
        gc_protected.emplace(ptr_);
        fprintf(stderr, "GC Protect: %p = %li\n", ptr_, loc_);
#endif
    }
}

scheme_value::~scheme_value() noexcept
{
    if (ptr_) {
#ifdef FLOW_GC_TRACE
        gc_protected.extract(ptr_);
        fprintf(stderr, "GC Release: %p = %li\n", ptr_, loc_);
        fprintf(stderr, "[ ");
        for (auto& p: gc_protected) {
            fprintf(stderr, "%p ", p);
        }
        fprintf(stderr, "]\n");
#endif
        s7_gc_unprotect_at(s7, loc_);
    }
    ptr_ = nullptr;
    loc_ = 0;
}

const char* scheme_value::pretty_print() const noexcept
{
    return ::flow::pretty_print(ptr_);
}

static s7_pointer prepare_pretty_print()
{
    s7_eval_c_string(s7, "(require write.scm)");
    return s7_eval_c_string(s7,
        "(lambda (obj)"
        "  (catch #t"
        "    (lambda () (pp obj))"
        "    (lambda (type info)"
        "      (apply format #f info))))");
}

const char* flow::pretty_print(s7_pointer obj)
{
    static s7_pointer pp = prepare_pretty_print();
    return s7_string(s7_call(s7, pp, s7_cons(s7, obj, s7_nil(s7))));
}

bool flow::scheme_add_resource_path(const char* path)
{
    auto curr = std::filesystem::current_path();
    auto full_path = curr;
    for (;;) {
        full_path = curr / path;
        if (std::filesystem::exists(full_path)) break;
        if (not curr.has_relative_path()) return false;
        curr = curr.parent_path();
    }

    s7_symbol_set_value(s7, s7_make_symbol(s7, "*load-path*"),
        s7_append(s7,
            s7_name_to_value(s7, "*load-path*"),
            s7_list(s7, 1, s7_make_string(s7, full_path.c_str()))
    ));

    return true;
}
