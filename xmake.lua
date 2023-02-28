add_requires("sdl2", "opengl", {system=true})
set_languages("c17", "c++20")

-- C/C++ flags
add_cxflags("-march=native", "-Wall", "-Wextra")

target("s7")
    set_kind("shared")
    add_cxflags("-g", "-O2")
    add_files("lib/s7/s7.c")

target("flow")
    add_rules("mode.debug", "mode.releasedbg", "mode.asan", "mode.ubsan")

    set_kind("binary")

    -- Output compile commands
    after_build(function (target)
        import("core.project.task")
        task.run("project", {kind = "compile_commands", outputdir="."})
    end)

    -- Imgui Library
    add_includedirs("lib/imgui", "lib/imgui/backends")
    add_files(
        "lib/imgui/*.cpp",
        "lib/imgui/misc/cpp/imgui_stdlib.cpp",
        "lib/imgui/backends/imgui_impl_opengl3.cpp",
        "lib/imgui/backends/imgui_impl_sdl.cpp")
    add_includedirs("lib/imnodes")
    add_files("lib/imnodes/*.cpp")

    -- Scheme (s7)
    add_includedirs("lib/s7")
    add_deps("s7")

    -- Other Libraries
    add_packages("sdl2", "opengl")

    -- Main
    add_files("src/*.cpp|test.cpp")

target("test")
    set_kind("binary")
    add_cxflags("-g", "-O0")

    -- Scheme (s7)
    add_includedirs("lib/s7")
    add_deps("s7")

    add_includedirs("lib/acutest/include")

    add_defines("FLOW_DISABLE_RUNTIME_WARNINGS")
    add_files("src/scheme_utils.cpp", "src/flow.cpp", "src/test.cpp")

target("nrepl")
    set_kind("binary")
    set_languages("gnu99")

    add_deps("s7")
    add_links("notcurses-core", "m", "dl")

    add_files("lib/s7/nrepl.c")
