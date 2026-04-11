use std::env;
use std::fs;
use std::path::{Path, PathBuf};

const CONFIG_SPECS: &[(&str, u32)] = &[
    ("VISION_UI_SCREEN_HEIGHT", 240),
    ("VISION_UI_SCREEN_WIDTH", 240),
    ("VISION_UI_ALLOW_EXIT_BY_USER", 0),
    ("VISION_UI_MAX_LIST_LAYER", 10),
    ("VISION_UI_EXIT_ANIMATION_DURATION_MS", 180),
    ("VISION_UI_ENTER_ANIMATION_DURATION_MS", 200),
    ("VISION_UI_NOTIFICATION_HEIGHT", 15),
    ("VISION_UI_NOTIFICATION_WIDTH", 20),
    ("VISION_UI_NOTIFICATION_DISMISS_DURATION_MS", 1500),
    ("VISION_UI_ALERT_HEIGHT", 20),
    ("VISION_UI_ALERT_WIDTH", 20),
    ("VISION_UI_LIST_ENTRY_ANIMATION", 0),
    ("VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING", 0),
    ("VISION_UI_LIST_TITLE_TO_FRAME_PADDING", 4),
    ("VISION_UI_LIST_FRAME_BETWEEN_PADDING", 2),
    ("VISION_UI_LIST_FOOTER_RIGHT_TO_SCROLL_BAR_PADDING", 10),
    ("VISION_UI_LIST_FOOTER_TO_LEFT_PADDING", 10),
    ("VISION_UI_LIST_HEADER_TO_TEXT_PADDING", 2),
    ("VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING", 4),
    ("VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING", 3),
    ("VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S", 5),
    ("VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS", 1500),
    ("VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS", 1000),
    ("VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S", 15),
    ("VISION_UI_LIST_FOOTER_MAX_HEIGHT", 11),
    ("VISION_UI_LIST_FOOTER_MAX_WIDTH", 19),
    ("VISION_UI_LIST_HEADER_MAX_WIDTH", 7),
    ("VISION_UI_LIST_SLIDER_FOOTER_WIDTH", 10),
    ("VISION_UI_LIST_FRAME_FIXED_HEIGHT", 15),
    ("VISION_UI_LIST_SELECTOR_FIXED_HEIGHT", 15),
    ("VISION_UI_ICON_VIEW_ITEM_SPACING", 15),
    ("VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING", 0),
    ("VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING", 10),
    ("VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING", 15),
    ("VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING", 5),
    ("VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING", 10),
    ("VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING", 20),
    ("VISION_UI_ICON_VIEW_DESCRIPTION_AREA_HEIGHT", 35),
    ("VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING", 3),
    ("VISION_UI_ICON_VIEW_SCROLL_SPEED", 85),
    ("VISION_UI_ICON_VIEW_ICON_SIZE", 100),
    ("VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT", 70),
    ("VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH", 9),
];

const FIXED_SPECS: &[(&str, u32)] = &[
    ("VISION_UI_LIST_SCROLL_BAR_WIDTH", 3),
    ("VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED", 92),
];

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Preset {
    Preset240x240,
    Preset128x64,
}

fn main() {
    let manifest_dir =
        PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("missing CARGO_MANIFEST_DIR"));
    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("missing OUT_DIR"));
    let source_dir = manifest_dir.join("../..");
    let selected = load_selected_config();
    let clang_args = selected_clang_args(&selected);

    for (name, _) in CONFIG_SPECS {
        println!("cargo:rerun-if-env-changed={name}");
    }
    println!("cargo:rerun-if-env-changed=VISION_UI_DEBUG");
    rerun_if_changed(&source_dir.join("include"));
    rerun_if_changed(&source_dir.join("src"));

    generate_selected_config(&out_dir, &selected, debug_enabled());
    generate_raw_bindings(&source_dir, &out_dir, &clang_args);
    compile_native(&source_dir, &clang_args);
}

fn load_selected_config() -> Vec<(&'static str, u32)> {
    let preset = selected_preset();

    CONFIG_SPECS
        .iter()
        .map(|(name, default)| {
            let preset_default = preset_override(preset, name).unwrap_or(*default);
            (*name, read_env_u32(name).unwrap_or(preset_default))
        })
        .collect()
}

fn selected_preset() -> Option<Preset> {
    let preset_240 = env::var_os("CARGO_FEATURE_PRESET_240X240").is_some();
    let preset_128 = env::var_os("CARGO_FEATURE_PRESET_128X64").is_some();

    match (preset_240, preset_128) {
        (true, true) => {
            panic!("features `preset-240x240` and `preset-128x64` are mutually exclusive")
        }
        (true, false) => Some(Preset::Preset240x240),
        (false, true) => Some(Preset::Preset128x64),
        (false, false) => None,
    }
}

fn preset_override(preset: Option<Preset>, name: &str) -> Option<u32> {
    match preset {
        Some(Preset::Preset240x240) => preset_240x240(name),
        Some(Preset::Preset128x64) => preset_128x64(name),
        None => None,
    }
}

fn preset_240x240(name: &str) -> Option<u32> {
    match name {
        "VISION_UI_SCREEN_WIDTH" => Some(240),
        "VISION_UI_SCREEN_HEIGHT" => Some(240),
        "VISION_UI_ICON_VIEW_ICON_SIZE" => Some(100),
        _ => None,
    }
}

fn preset_128x64(name: &str) -> Option<u32> {
    match name {
        "VISION_UI_SCREEN_WIDTH" => Some(128),
        "VISION_UI_SCREEN_HEIGHT" => Some(64),
        "VISION_UI_NOTIFICATION_HEIGHT" => Some(12),
        "VISION_UI_NOTIFICATION_WIDTH" => Some(18),
        "VISION_UI_ALERT_HEIGHT" => Some(16),
        "VISION_UI_ALERT_WIDTH" => Some(18),
        "VISION_UI_LIST_TITLE_TO_FRAME_PADDING" => Some(3),
        "VISION_UI_LIST_FRAME_BETWEEN_PADDING" => Some(1),
        "VISION_UI_LIST_FOOTER_RIGHT_TO_SCROLL_BAR_PADDING" => Some(6),
        "VISION_UI_LIST_FOOTER_TO_LEFT_PADDING" => Some(5),
        "VISION_UI_LIST_HEADER_TO_TEXT_PADDING" => Some(2),
        "VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING" => Some(2),
        "VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING" => Some(2),
        "VISION_UI_LIST_FOOTER_MAX_HEIGHT" => Some(9),
        "VISION_UI_LIST_FOOTER_MAX_WIDTH" => Some(12),
        "VISION_UI_LIST_HEADER_MAX_WIDTH" => Some(6),
        "VISION_UI_LIST_SLIDER_FOOTER_WIDTH" => Some(8),
        "VISION_UI_LIST_FRAME_FIXED_HEIGHT" => Some(12),
        "VISION_UI_LIST_SELECTOR_FIXED_HEIGHT" => Some(12),
        "VISION_UI_ICON_VIEW_ITEM_SPACING" => Some(8),
        "VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING" => Some(6),
        "VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING" => Some(8),
        "VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING" => Some(3),
        "VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING" => Some(6),
        "VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING" => Some(8),
        "VISION_UI_ICON_VIEW_DESCRIPTION_AREA_HEIGHT" => Some(18),
        "VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING" => Some(2),
        "VISION_UI_ICON_VIEW_SCROLL_SPEED" => Some(64),
        "VISION_UI_ICON_VIEW_ICON_SIZE" => Some(36),
        "VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT" => Some(20),
        "VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH" => Some(6),
        _ => None,
    }
}

fn selected_clang_args(selected: &[(&str, u32)]) -> Vec<String> {
    selected
        .iter()
        .map(|(name, value)| format!("-DCONFIG_{name}={value}"))
        .collect()
}

fn generate_selected_config(out_dir: &Path, selected: &[(&str, u32)], debug: bool) {
    let mut contents = String::new();
    for (name, value) in selected {
        contents.push_str(&format!("pub const {name}: u32 = {value};\n"));
    }
    for (name, value) in FIXED_SPECS {
        contents.push_str(&format!("pub const {name}: u32 = {value};\n"));
    }
    contents.push_str(&format!(
        "pub const DEBUG_OVERLAY: bool = {};\n",
        if debug { "true" } else { "false" }
    ));
    fs::write(out_dir.join("selected_config.rs"), contents)
        .expect("failed to write selected config");
}

fn generate_raw_bindings(source_dir: &Path, out_dir: &Path, clang_args: &[String]) {
    let header = source_dir.join("include/vision/vision_ui.h");
    let bindings = bindgen::Builder::default()
        .header(header.display().to_string())
        .allowlist_function("vision_ui_.*")
        .allowlist_type("vision_.*")
        .allowlist_var(
            "IS_IN_VISION_UI|DEFAULT_LIST_ICON|UiAction.*|ListItem|TitleItem|IconItem|SwitchItem|SliderItem|UserItem|VisionAlloc.*",
        )
        .merge_extern_blocks(true)
        .use_core()
        .clang_arg(format!("-I{}", source_dir.join("include").display()))
        .clang_args(clang_args.iter().map(String::as_str))
        .generate()
        .expect("failed to generate raw bindings");

    bindings
        .write_to_file(out_dir.join("raw_bindings.rs"))
        .expect("failed to write raw bindings");
}

fn compile_native(source_dir: &Path, clang_args: &[String]) {
    let mut build = cc::Build::new();
    build
        .include(source_dir.join("include"))
        .include(source_dir.join("include/vision"))
        .include(source_dir.join("src"))
        .std("c99");

    for arg in clang_args {
        if let Some(define) = arg.strip_prefix("-D") {
            let mut parts = define.splitn(2, '=');
            let key = parts.next().expect("missing define key");
            let value = parts.next().unwrap_or("1");
            build.define(key, Some(value));
        }
    }
    if debug_enabled() {
        build.define("DEBUG", None);
    }

    for source in [
        "src/vision_ui_animation.c",
        "src/vision_ui_core.c",
        "src/vision_ui_item.c",
        "src/vision_ui_renderer.c",
    ] {
        build.file(source_dir.join(source));
    }

    build.compile("vision_ui_native");
}

fn rerun_if_changed(root: &Path) {
    println!("cargo:rerun-if-changed={}", root.display());
}

fn read_env_u32(name: &str) -> Option<u32> {
    env::var(name).ok().map(|value| {
        value
            .parse::<u32>()
            .unwrap_or_else(|_| panic!("invalid {name} value: {value}"))
    })
}

fn debug_enabled() -> bool {
    env::var_os("CARGO_FEATURE_DEBUG_OVERLAY").is_some()
        || read_env_bool("VISION_UI_DEBUG").unwrap_or(false)
}

fn read_env_bool(name: &str) -> Option<bool> {
    env::var(name).ok().map(|value| match value.as_str() {
        "1" | "true" | "TRUE" | "yes" | "YES" | "on" | "ON" => true,
        "0" | "false" | "FALSE" | "no" | "NO" | "off" | "OFF" => false,
        _ => panic!("invalid {name} value: {value}"),
    })
}
