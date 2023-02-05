add_requires("sdl2", "opengl", {system=true})
set_languages("c18", "c++20")

-- Debug
add_cxflags("-g3", "-Og", "-march=native")

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
        "lib/imgui/backends/imgui_impl_opengl3.cpp", 
        "lib/imgui/backends/imgui_impl_sdl.cpp")

    -- Other Libraries
    add_packages("sdl2", "opengl")

    -- Main
    add_files("src/*.cpp")
