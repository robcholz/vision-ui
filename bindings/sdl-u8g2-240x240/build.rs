use std::path::{Path, PathBuf};

fn main() {
    let manifest_dir =
        PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").expect("missing CARGO_MANIFEST_DIR"));

    let layout = resolve_layout(&manifest_dir);
    for watched in layout.rerun_paths() {
        println!("cargo:rerun-if-changed={}", watched.display());
    }

    generate_sim_bindings(&layout);
    compile_simulator_240x240(&layout);
}

struct BuildLayout {
    driver_dir: PathBuf,
    include_dir: PathBuf,
    vision_include_dir: PathBuf,
    u8g2_dir: PathBuf,
    sdl_common_dir: PathBuf,
}

impl BuildLayout {
    fn rerun_paths(&self) -> [PathBuf; 5] {
        [
            self.driver_dir.clone(),
            self.include_dir.clone(),
            self.vision_include_dir.clone(),
            self.u8g2_dir.clone(),
            self.sdl_common_dir.clone(),
        ]
    }
}

fn resolve_layout(manifest_dir: &Path) -> BuildLayout {
    let repo_dir = manifest_dir.parent().and_then(Path::parent);
    if let Some(repo_dir) = repo_dir {
        let driver_dir = repo_dir.join("src/driver");
        let include_dir = repo_dir.join("include");
        let vision_include_dir = include_dir.join("vision");
        let u8g2_dir = repo_dir.join("components/u8g2/csrc");
        let sdl_common_dir = repo_dir.join("components/u8g2/sys/sdl/common");

        if driver_dir.join("u8g2.c").exists()
            && vision_include_dir.join("vision_ui.h").exists()
            && u8g2_dir.join("u8g2.h").exists()
            && sdl_common_dir.join("u8x8_sdl_key.c").exists()
        {
            return BuildLayout {
                driver_dir,
                include_dir,
                vision_include_dir,
                u8g2_dir,
                sdl_common_dir,
            };
        }
    }

    let native_dir = manifest_dir.join("native");
    BuildLayout {
        driver_dir: native_dir.join("driver"),
        include_dir: native_dir.join("include"),
        vision_include_dir: native_dir.join("include/vision"),
        u8g2_dir: native_dir.join("vendor/u8g2/csrc"),
        sdl_common_dir: native_dir.join("sdl_common"),
    }
}

fn compile_simulator_240x240(layout: &BuildLayout) {
    let (sdl_cflags, sdl_libs) = sdl2_config();

    let mut build = cc::Build::new();
    build
        .include(layout.include_dir.as_path())
        .include(layout.vision_include_dir.as_path())
        .include(layout.u8g2_dir.as_path())
        .include(layout.driver_dir.as_path())
        .include(layout.sdl_common_dir.as_path())
        .std("c99");

    for flag in sdl_cflags.split_whitespace() {
        if let Some(include) = flag.strip_prefix("-I") {
            build.include(include);
        } else {
            build.flag(flag);
        }
    }

    build.file(layout.driver_dir.join("u8g2_240x240.c"));
    build.file(layout.sdl_common_dir.join("u8x8_sdl_key.c"));

    for source in u8g2_sources(&layout.u8g2_dir) {
        build.file(source);
    }

    build.compile("vision_ui_simulator_240x240_native");

    for flag in sdl_libs.split_whitespace() {
        if let Some(search) = flag.strip_prefix("-L") {
            println!("cargo:rustc-link-search=native={search}");
        } else if let Some(lib) = flag.strip_prefix("-l") {
            println!("cargo:rustc-link-lib={lib}");
        }
    }
}

fn generate_sim_bindings(layout: &BuildLayout) {
    let out_dir = PathBuf::from(std::env::var("OUT_DIR").expect("missing OUT_DIR"));
    let header = "#include <u8g2.h>\n#include \"u8g2_240x240_display.h\"\n";

    let mut builder = bindgen::Builder::default()
        .header_contents("vision_ui_simulator_bindings.h", header)
        .allowlist_type("u8g2_t")
        .allowlist_type("u8x8_t")
        .allowlist_type("u8g2_cb_t")
        .allowlist_function("u8x8_setup_sdl_240x240")
        .allowlist_function("u8g2_SetupBuffer")
        .allowlist_function("u8x8_InitDisplay")
        .allowlist_function("u8x8_SetPowerSave")
        .allowlist_function("u8g2_ll_hvline_vertical_top_lsb")
        .allowlist_var("u8g2_cb_r0")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .use_core();

    for include_dir in [
        layout.include_dir.as_path(),
        layout.vision_include_dir.as_path(),
        layout.u8g2_dir.as_path(),
        layout.driver_dir.as_path(),
        layout.sdl_common_dir.as_path(),
    ] {
        builder = builder.clang_arg(format!("-I{}", include_dir.display()));
    }

    builder
        .generate()
        .unwrap_or_else(|err| panic!("failed to generate simulator bindings: {err}"))
        .write_to_file(out_dir.join("sim_bindings.rs"))
        .unwrap_or_else(|err| panic!("failed to write simulator bindings: {err}"));
}

fn sdl2_config() -> (String, String) {
    let cflags = run_command("sdl2-config", "--cflags");
    let libs = run_command("sdl2-config", "--libs");
    (cflags, libs)
}

fn run_command(program: &str, arg: &str) -> String {
    let output = std::process::Command::new(program)
        .arg(arg)
        .output()
        .unwrap_or_else(|err| panic!("failed to run `{program} {arg}`: {err}"));
    if !output.status.success() {
        panic!(
            "`{program} {arg}` failed: {}",
            String::from_utf8_lossy(&output.stderr)
        );
    }
    String::from_utf8(output.stdout)
        .unwrap_or_else(|err| panic!("`{program} {arg}` did not return UTF-8 output: {err}"))
        .trim()
        .to_owned()
}

fn u8g2_sources(root: &Path) -> Vec<PathBuf> {
    let mut files = std::fs::read_dir(root)
        .unwrap_or_else(|err| panic!("failed to read u8g2 sources from {}: {err}", root.display()))
        .filter_map(|entry| {
            let path = entry.ok()?.path();
            if path.extension().and_then(|ext| ext.to_str()) == Some("c") {
                Some(path)
            } else {
                None
            }
        })
        .collect::<Vec<_>>();
    files.sort();
    files
}
