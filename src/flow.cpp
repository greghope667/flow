#include "flow.h"

#include "flow_assert.h"
#include <s7.h>

using namespace flow;

function_id_t scene::add_function() {
    functions_.emplace_back();
    return function_id_t(functions_.size() - 1);
}

function& scene::get_function(function_id_t idx) {
    return functions_.at(int(idx));
}

port_id_t scene::add_input(function_id_t parent) {
    port p  ={
        .parent = parent,
        .is_input = true,
    };
    ports_.push_back(p);
    auto id = port_id_t(ports_.size() - 1);
    get_function(parent).inputs.push_back(id);
    return id;
}

port_id_t scene::add_output(function_id_t parent) {
    port p  ={
        .parent = parent,
        .is_input = false,
    };
    ports_.push_back(p);
    auto id = port_id_t(ports_.size() - 1);
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
        .source = src,
        .dest = dst,
    };
    pipes_.push_back(p);
    auto id = pipe_id_t(pipes_.size() - 1);
    get_port(src).pipes.push_back(id);
    get_port(dst).pipes.push_back(id);
    return id;
}

flow::pipe& scene::get_pipe(pipe_id_t idx) {
    return pipes_.at(int(idx));
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

    for (auto& func: functions_) {
        if (can_run_safely(func)) {
            exec(func);
            return true;
        }
    }
    return false;
}