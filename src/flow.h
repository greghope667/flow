#pragma once

#include <string>
#include <vector>
#include "scheme_utils.h"
#include "vector_pool.h"

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
enum class port_id_t : uint32_t;
enum class function_id_t : uint32_t;
enum class pipe_id_t : uint32_t;

struct port {
    std::vector<pipe_id_t> pipes;
    std::string name;
    function_id_t parent;
    bool is_input;
};

struct function {
    std::string code;
    std::string result;
    std::vector<port_id_t> inputs;
    std::vector<port_id_t> outputs;
};

struct pipe {
    scheme_value data;
    port_id_t source;
    port_id_t dest;
};

// The main resource holder

class scene {
public:
    scene() = default;

    // Accessors ... possibly too many
    function_id_t add_function();
    function& get_function(function_id_t);
    void remove_function(function_id_t);
    auto all_functions() const { return functions_.valid_indexes(); }

    port_id_t add_input(function_id_t parent);
    port_id_t add_output(function_id_t parent);
    port& get_port(port_id_t);
    void remove_port(port_id_t);

    pipe_id_t add_pipe(port_id_t src, port_id_t dst);
    pipe& get_pipe(pipe_id_t);
    void remove_pipe(pipe_id_t);
    auto all_pipes() const { return pipes_.valid_indexes(); }

    bool exec_step();
    std::string exec(function& func, bool prints = false);

private:
    //bool is_pipe_full(const pipe& p);
    //bool is_input_ready(const port& input);
    //bool is_output_ready(const port& output);
    //bool is_function_ready(const function& func);

private:
    vector_pool<function> functions_;
    vector_pool<port> ports_;
    vector_pool<pipe> pipes_;
};


}
