set_project("vision_ui")
set_xmakever("2.8.2")
set_languages("c99", "cxx23")
add_rules("mode.debug")
add_requires("libsdl >=2.0", { configs = { shared = true } })

option("simulator")
set_default(true)
set_showmenu(true)
set_description("Build SDL simulator for vision-ui")
option_end()

if has_config("simulator") then
    target("u8g2")
    set_kind("static")
    add_files("components/u8g2/csrc/*.c")
    add_includedirs("components/u8g2/csrc", "components/u8g2/cppsrc", { public = true })
end

target("vision_ui")
set_kind("static")
add_files("src/*.c")
add_includedirs("include", "include/vision", { public = true })

if has_config("simulator") then
    target("vision_ui_simulator")
    set_kind("binary")
    add_files(
            "main.cpp",
            "components/u8g2/sys/sdl/common/u8x8_sdl_key.c"
    )
    add_files("src/driver/*.c")
    add_includedirs(
            "components/u8g2/csrc",
            "components/u8g2/sys/sdl/common",
            "src"
    )
    add_deps("u8g2")
    add_deps("vision_ui")
    add_packages("libsdl")
end
