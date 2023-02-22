#pragma once

#include <string>
#include <vector>
#include "scheme_utils.h"

namespace flow {

/* Data Structures
 * `function`s have inputs `in_port` and outputs `out_port`
 * `pipe`s connect from function outputs back to function inputs
 *
 * All data is held by a `scene`, which manages connections and execution
 *
 * TODO: std::vector should be some SBO vector type, as most objects will
 * have 0 or 1 connections
 */

// Hack for distinct int types
enum class in_port_id_t : uint32_t;
enum class out_port_id_t : uint32_t;
enum class function_id_t : uint32_t;
enum class pipe_id_t : uint32_t;

struct in_port {
    std::vector<pipe_id_t> pipes;
    std::string name;
    function_id_t parent;
    bool ready;
};

struct out_port {
    std::vector<pipe_id_t> pipes;
    std::string name;
    function_id_t parent;
};

struct function {
    std::string code;
    std::vector<in_port_id_t> inputs;
    std::vector<out_port_id_t> outputs;
};

struct pipe {
    scheme_value data;
    out_port_id_t source;
    in_port_id_t dest;
};

// The main resource holder

class scene {
public:
    scene() = default;

    // Accessors ... possibly too many
    function_id_t add_function();
    function& get_function(function_id_t);
    void remove_function(function_id_t);
    const auto& all_functions() const { return functions_; }

    in_port_id_t add_input(function_id_t parent);
    in_port& get_input(in_port_id_t);
    void remove_input(in_port_id_t);

    out_port_id_t add_output(function_id_t parent);
    out_port& get_output(out_port_id_t);
    void remove_output(out_port_id_t);

    pipe_id_t add_pipe(out_port_id_t src, in_port_id_t dst);
    pipe& get_pipe(pipe_id_t);
    void remove_pipe(pipe_id_t);

    bool exec_step();

private:
    std::vector<function> functions_;
    std::vector<in_port> inputs_;
    std::vector<out_port> outputs_;
    std::vector<pipe> pipes_;
};


}