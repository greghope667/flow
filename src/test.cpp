#include <acutest.h>
#include "flow.h"

void test_empty(void)
{}

static void setup_scheme_env()
{
    flow::scheme_init();
    flow::scheme_add_resource_path("lib/s7");
}

void test_scene()
{
    setup_scheme_env();
    flow::scene x{};
    auto f1id = x.add_function();
    x.get_function(f1id).code = "(define x 7)";

    auto f2id = x.add_function();
    x.get_function(f2id).code = "(define y (+ x 3))";

    auto f1outid = x.add_output(f1id);
    x.get_port(f1outid).name = "x";

    auto f2inid = x.add_input(f2id);
    x.get_port(f2inid).name = "x";

    auto pipeid = x.add_pipe(f1outid, f2inid);

    TEST_CHECK(x.exec(x.get_function(f1id)) == "7");
    TEST_CHECK(x.get_pipe(pipeid).data);
    TEST_CHECK(x.exec(x.get_function(f2id)) == "10");
}

void test_undefined()
{
    setup_scheme_env();
    flow::scene x{};
    auto f1id = x.add_function();
    x.get_function(f1id).code = "3";

    auto f2id = x.add_function();

    auto f1outid = x.add_output(f1id);
    x.get_port(f1outid).name = "x";

    auto f2inid = x.add_input(f2id);
    x.get_port(f2inid).name = "x";

    auto pipeid = x.add_pipe(f1outid, f2inid);

    TEST_CHECK(x.exec(x.get_function(f1id)) == "3");
    TEST_CHECK(not x.get_pipe(pipeid).data);
    x.exec(x.get_function(f2id));
}

TEST_LIST = {
    { "trivial", test_empty },
    { "simple scene", test_scene },
    { "undefined data", test_undefined },
    { nullptr, nullptr },
};
