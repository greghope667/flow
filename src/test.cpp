#include <acutest.h>
#include <algorithm>
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

    /* Enough loops for GC to kick in */
    for (int i=0; i<100; i++) {
	    TEST_ASSERT(x.exec(x.get_function(f1id)) == "7");
	    TEST_ASSERT(x.get_pipe(pipeid).data);
	    TEST_ASSERT(x.exec(x.get_function(f2id)) == "10");
    }
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

void test_pool_insert_remove()
{
    flow::vector_pool<char> pool{};
    TEST_ASSERT(pool.vector().size() == 0);
    auto idx = pool.acquire_object();
    TEST_ASSERT(pool.vector().size() > 0);
    TEST_ASSERT((pool.at(idx) = '1') == '1');
    TEST_ASSERT(pool.at(idx) == '1');
    pool.release_object(idx);
    TEST_EXCEPTION(pool.at(idx), std::exception);
}

void test_pool_iteration()
{
    flow::vector_pool<char> pool{};
    pool.at(pool.acquire_object()) = '1';
    auto mid = pool.acquire_object();
    pool.at(mid) = '2';
    pool.at(pool.acquire_object()) = '3';
    pool.at(pool.acquire_object()) = '4';
    pool.release_object(mid);

    auto vi = [&]() -> std::vector<int> {
        auto vi = pool.valid_indexes();
        return {vi.begin(), vi.end()};
    }();

    TEST_CHECK(vi.size() == 3);

    std::vector<char> remaining;
    for (int i: vi) {
        remaining.push_back(pool.at(i));
    }

    // Ordering not class requirement - run sort so we can test equality
    std::sort(remaining.begin(), remaining.end());

    std::vector<char> required = {'1','3','4'};
    TEST_CHECK(required == remaining);
}

TEST_LIST = {
    { "trivial", test_empty },
    { "simple scene", test_scene },
    { "undefined data", test_undefined },
    { "pool insertion", test_pool_insert_remove },
    { "pool iteration", test_pool_iteration },
    { nullptr, nullptr },
};
