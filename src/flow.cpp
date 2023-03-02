#include "flow.h"

#include "flow_assert.h"
#include <s7.h>

using namespace flow;

function_id_t scene::add_function() {
    return function_id_t(functions_.acquire_object());
}

function& scene::get_function(function_id_t idx) {
    return functions_.at(int(idx));
}

port_id_t scene::add_input(function_id_t parent) {
    port p  ={
        .pipes = {},
        .name = "",
        .parent = parent,
        .is_input = true,
    };

    auto id = port_id_t(ports_.acquire_object());
    ports_.at(int(id)) = std::move(p);
    get_function(parent).inputs.push_back(id);

    return id;
}

port_id_t scene::add_output(function_id_t parent) {
    port p  ={
        .pipes = {},
        .name = "",
        .parent = parent,
        .is_input = false,
    };

    auto id = port_id_t(ports_.acquire_object());
    ports_.at(int(id)) = std::move(p);
    get_function(parent).outputs.push_back(id);

    return id;
}

port& scene::get_port(port_id_t idx) {
    return ports_.at(int(idx));
}

pipe_id_t scene::add_pipe(port_id_t src, port_id_t dst) {
    throw_assert(get_port(src).is_input == false, "port should be output type");
    throw_assert(get_port(dst).is_input == true, "port should be input type");

    pipe p = {
        .data = nullptr,
        .source = src,
        .dest = dst,
    };

    auto id = pipe_id_t(pipes_.acquire_object());
    pipes_.at(int(id)) = std::move(p);
    get_port(src).pipes.push_back(id);
    get_port(dst).pipes.push_back(id);

    return id;
}

flow::pipe& scene::get_pipe(pipe_id_t idx) {
    return pipes_.at(int(idx));
}

void scene::remove_function(function_id_t idx) {
    auto& function = get_function(idx);

    // Remove children
    for (auto port_id: function.inputs) {
        remove_port(port_id);
    }
    for (auto port_id: function.outputs) {
        remove_port(port_id);
    }

    // Remove self
    functions_.release_object(int(idx));
}

void scene::remove_port(port_id_t idx) {
    auto& port = get_port(idx);

    // Remove children
    for (auto pipe_id: port.pipes) {
        remove_pipe(pipe_id);
    }

    // Unassign from parent
    auto& parent = get_function(port.parent);
    if (port.is_input) {
        std::erase(parent.inputs, idx);
    } else {
        std::erase(parent.outputs, idx);
    }

    // Remove self
    ports_.release_object(int(idx));
}

void scene::remove_pipe(pipe_id_t idx) {
    auto& pipe = get_pipe(idx);

    // Unassign from parents
    auto& src = get_port(pipe.source);
    auto& dst = get_port(pipe.dest);
    std::erase(src.pipes, idx);
    std::erase(dst.pipes, idx);

    // Remove self
    pipes_.release_object(int(idx));
}

std::string scene::exec(function& func, bool prints)
{
    scheme_value defs = s7_list(s7, 0);

    for (auto input_id: func.inputs) {
        auto& input = get_port(input_id);
        throw_assert(input.is_input, "input port marked as output");
        throw_assert(&func == &get_function(input.parent), "bad parent");

        scheme_value arg_value{};

        for (auto link_id: input.pipes) {
            auto& link = get_pipe(link_id);
            throw_assert(link.dest == input_id, "pipe target mismatch");

            if (link.data) {
                warn_assert(not arg_value, "multipe data for same input");
                arg_value = std::move(link.data);
            }
        }

        warn_assert(arg_value, "missing data for input");

        if (arg_value)
            defs = s7_cons(s7,
                s7_cons(s7, s7_make_symbol(s7, input.name.c_str()), arg_value.get()),
                defs.get()
            );
    }

    throw_assert(s7_is_proper_list(s7, defs.get()), "arguments must be a list");

    scheme_value env = s7_inlet(s7, defs.get());

    if (prints)
        printf("Before: %s\n", env.pretty_print());

    scheme_value result = s7_eval_c_string_with_environment(s7, func.code.c_str(), env.get());
    if (prints) {
        printf("result = %s\n", result.pretty_print());
        printf("After: %s\n", env.pretty_print());
        fflush(stdout);
    }

    for (auto output_id: func.outputs) {
        auto& output = get_port(output_id);
        throw_assert(not output.is_input, "output port marked as input");
        throw_assert(&func == &get_function(output.parent), "bad parent");

        scheme_value output_data;

        if (output.name.empty()) {
            output_data = result;
        } else {
            auto sym = s7_make_symbol(s7, output.name.c_str());
            auto val = s7_symbol_local_value(s7, sym, env.get());
            if (s7_is_eq(val, s7_undefined(s7))) {
                output_data = nullptr;
            } else {
                output_data = val;
            }
        }

        for (auto link_id: output.pipes) {
            auto& link = get_pipe(link_id);
            throw_assert(link.source == output_id, "pipe source target");
            warn_assert(not link.data, "overwriting output data");

            link.data = output_data;
        }
    }

    return result.pretty_print();
}

#include <algorithm>
bool scene::exec_step() {
    auto has_data = [&](auto pipe_id) {
        return bool(get_pipe(pipe_id).data);
    };

    auto can_run_safely = [&](auto& func){
        for (auto input_id: func.inputs) {
            auto& input = get_port(input_id);

            if (not std::ranges::any_of(input.pipes, has_data)) {
                return false;
            }
        }

        for (auto output_id: func.outputs) {
            auto& output = get_port(output_id);

            if (std::ranges::any_of(output.pipes, has_data)) {
                return false;
            }
        }

        return true;
    };

    for (auto id: functions_.valid_indexes()) {
        auto& func = get_function(function_id_t(id));
        if (can_run_safely(func)) {
            exec(func);
            return true;
        }
    }
    return false;
}
