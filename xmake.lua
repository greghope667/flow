add_requires("sdl2", "opengl", {system=true})
set_languages("c18", "c++20")


add_rules("mode.debug", "mode.release")

-- C/C++ flags
add_cxflags("-march=native", "-Wall", "-Wextra")
if is_mode("debug") then
    add_cxflags("-g3", "-Og")
end

target("flow")
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
    add_files("lib/s7/s7.c")

    -- Other Libraries
    add_packages("sdl2", "opengl")

    -- Main
    add_files("src/*.cpp")
