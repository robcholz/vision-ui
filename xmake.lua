set_project("vision_ui")
set_languages("c99", "cxx23")
add_rules("mode.debug")

add_requires("libsdl >=2.0", { configs = { shared = true } })

option("language")
   set_showmenu(true)
   set_description("Select simulator language")
   set_default("default")
   set_values("default", "english")
option_end()

target("u8g2")
    set_kind("static")
    add_files("components/u8g2/csrc/*.c")
    add_includedirs("components/u8g2/csrc", "components/u8g2/cppsrc", { public = true })

target("vision_ui")
    set_kind("static")
    add_files("include/vision/*.c")
    add_files("include/driver/*.c")
    add_includedirs("include", { public = true })
    add_deps("u8g2")
    add_packages("libsdl")


target("vision_ui_simulator")
    set_kind("binary")
    local simulator_main = "main_english.cpp"
    add_files(
        simulator_main,
        "components/u8g2/sys/sdl/common/u8x8_d_sdl_128x64.c",
        "components/u8g2/sys/sdl/common/u8x8_sdl_key.c"
    )
    add_includedirs("components/u8g2/csrc", "components/u8g2/sys/sdl/common")
    add_deps("u8g2")
    add_deps("vision_ui")
    add_packages("libsdl")