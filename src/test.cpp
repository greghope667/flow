#include <acutest.h>
#include <algorithm>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
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
    flow::scheme_free();
}

void test_reversed_pipe()
{
    setup_scheme_env();
    flow::scene x{};

    auto f1id = x.add_function();
    auto f2id = x.add_function();
    auto f1outid = x.add_output(f1id);
    auto f2inid = x.add_input(f2id);

    TEST_EXCEPTION(x.add_pipe(f2inid, f1outid), std::exception);
    TEST_EXCEPTION(x.add_pipe(f1outid, f1outid), std::exception);
    TEST_EXCEPTION(x.add_pipe(f2inid, f2inid), std::exception);
    flow::scheme_free();
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
    flow::scheme_free();
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

    auto vi = ranges::to_vector(pool.valid_indexes());
    TEST_CHECK(vi.size() == 3);

    auto remaining = ranges::to_vector(vi | ranges::views::transform([&](int i){return pool.at(i);}));
    TEST_CHECK(remaining.size() == 3);

    // Ordering not class requirement - run sort so we can test equality
    std::sort(remaining.begin(), remaining.end());

    std::vector<char> required = {'1','3','4'};
    TEST_CHECK(required == remaining);
}

void test_scene_removal()
{
    flow::scene x{};
    auto f1id = x.add_function();
    auto f2id = x.add_function();
    auto f1outid = x.add_output(f1id);
    auto f2inid = x.add_input(f2id);
    auto pipeid = x.add_pipe(f1outid, f2inid);

    TEST_CHECK(ranges::to_vector(x.all_pipes()).size() == 1);
    x.remove_pipe(pipeid);
    TEST_CHECK(ranges::to_vector(x.all_pipes()).size() == 0);
    x.add_pipe(f1outid, f2inid);
    TEST_CHECK(ranges::to_vector(x.all_pipes()).size() == 1);
    x.remove_port(f1outid);
    TEST_CHECK(ranges::to_vector(x.all_pipes()).size() == 0);
    TEST_CHECK(ranges::to_vector(x.all_functions()).size() == 2);
    x.remove_function(f1id);
    TEST_CHECK(ranges::to_vector(x.all_functions()).size() == 1);
    TEST_CHECK(ranges::to_vector(x.all_pipes()).size() == 0);
}

TEST_LIST = {
    { "trivial", test_empty },
    { "simple scene", test_scene },
    { "reversed pipe", test_reversed_pipe },
    { "undefined data", test_undefined },
    { "pool insertion", test_pool_insert_remove },
    { "pool iteration", test_pool_iteration },
    { "scene item removal", test_scene_removal },
    { nullptr, nullptr },
};
