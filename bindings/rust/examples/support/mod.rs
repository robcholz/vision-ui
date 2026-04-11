use core::ffi::c_void;
use core::mem::{align_of, size_of};
use core::ptr::{self, NonNull};
use std::alloc::{Layout, alloc, alloc_zeroed, dealloc};
use std::sync::Mutex;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::time::Duration;
use vision_ui::{
    AllocRequest, Allocator, MonoBitmap, SceneBindings, SlideBinding, StartupLogo, ToggleBinding,
    UiRef, VisionUi, text,
};

#[repr(C, align(16))]
struct VisionAllocHeader {
    size: usize,
}

static ALLOC_TOTAL: AtomicUsize = AtomicUsize::new(0);
static UNIT: () = ();
static SCENE_STATE: Mutex<SceneState> = Mutex::new(SceneState::new());

static NOTIFY_TEST_1: ToggleBinding<()> = ToggleBinding::new(&UNIT, notify_test_1);
static NOTIFY_TEST_2: ToggleBinding<()> = ToggleBinding::new(&UNIT, notify_test_2);
static ALERT_TEST: ToggleBinding<()> = ToggleBinding::new(&UNIT, alert_test);
static NOOP_SWITCH: ToggleBinding<()> = ToggleBinding::new(&UNIT, noop_switch_changed);
static NOOP_SLIDER: SlideBinding<()> = SlideBinding::new(&UNIT, noop_slider_changed);
static ABOUT_SCENE: SceneBindings<Mutex<SceneState>> = SceneBindings::new(
    &SCENE_STATE,
    Some(scene_init),
    Some(scene_render),
    Some(scene_exit),
);

static ICON_BITMAP_100X100: [u8; 1300] = [0; 1300];
static STARTUP_LOGO: [u8; 7200] = [0; 7200];

struct LoggingAllocator;

unsafe impl Allocator for LoggingAllocator {
    fn allocate(request: AllocRequest) -> *mut c_void {
        match request {
            AllocRequest::Malloc { size } => alloc_with_header(size, false),
            AllocRequest::Calloc { size, count } => size
                .checked_mul(count)
                .map(|payload| alloc_with_header(payload, true))
                .unwrap_or_else(|| {
                    eprintln!("calloc overflow: size={size}, count={count}");
                    ptr::null_mut()
                }),
            AllocRequest::Free { ptr } => {
                free_with_header(ptr);
                ptr::null_mut()
            }
        }
    }
}

#[derive(Clone, Copy)]
struct SceneState;

impl SceneState {
    const fn new() -> Self {
        Self
    }

    fn reset(&mut self) {
        *self = Self::new();
    }
}

pub fn run_demo<D, F>(driver: D, setup: F) -> Result<(), Box<dyn std::error::Error>>
where
    D: vision_ui::driver::Driver + 'static,
    F: FnOnce(&mut VisionUi<D>) -> Result<(), Box<dyn std::error::Error>>,
{
    let mut ui = VisionUi::new(driver);
    ui.set_allocator::<LoggingAllocator>();
    setup(&mut ui)?;

    let icon_bitmap = MonoBitmap::new(&ICON_BITMAP_100X100, 100, 100)?;
    let startup_logo = StartupLogo::new(&STARTUP_LOGO)?;

    let root = ui.list(text!("VisionUI"), 16)?;
    ui.set_root(root)?;

    let board_settings = ui.list(text!("Board Settings"), 5)?;
    let board_settings_2 = ui.icon_list(text!("Board Settings 2"), 3)?;

    let icon_1 = ui.icon(text!("Icon 1"), Some(text!("Example Icon 1")), None, 0)?;
    ui.push(board_settings_2, icon_1)?;
    let icon_2 = ui.icon(
        text!("Icon Super Looooooooong"),
        Some(text!("Example Icon 2")),
        Some(icon_bitmap),
        0,
    )?;
    ui.push(board_settings_2, icon_2)?;
    let icon_item_3 = ui.icon(text!("Icon Item 3"), None, None, 1)?;
    let board_settings_3 = ui.list(text!("Board Settings3"), 0)?;
    ui.push(icon_item_3, board_settings_3)?;
    ui.push(board_settings_2, icon_item_3)?;

    let root_title = ui.title(text!("VisionUI"))?;
    ui.push(root, root_title)?;
    ui.push(root, board_settings)?;
    ui.push(root, board_settings_2)?;
    let about_scene = ui.scene_with(text!("About the Board..."), &ABOUT_SCENE)?;
    ui.push(root, about_scene)?;

    for (label, binding) in [
        (text!("Test Notification 1"), &NOTIFY_TEST_1),
        (text!("Test Notification 2"), &NOTIFY_TEST_2),
        (text!("Test Alert"), &ALERT_TEST),
        (
            text!("Test Notification 1 Notification Test 2 Notification Test 2"),
            &NOTIFY_TEST_1,
        ),
        (
            text!("Test Notification 2 Notification Test 2 Notification Test 2"),
            &NOTIFY_TEST_2,
        ),
        (
            text!("Test Alert Notification Test 2 Notification Test 2"),
            &ALERT_TEST,
        ),
        (
            text!("Test Notification 1 Notification Test 2 Notification Test 2"),
            &NOTIFY_TEST_1,
        ),
        (
            text!("Test Notification 2Notification Test 2 Notification Test 2"),
            &NOTIFY_TEST_2,
        ),
        (
            text!("Test Alert Notification Test 2 Notification Test 2"),
            &ALERT_TEST,
        ),
        (
            text!("Test Notification 1 Notification Test 2 Notification Test 2"),
            &NOTIFY_TEST_1,
        ),
        (
            text!("Test Notification 2 Notification Test 2 Notification Test 2"),
            &NOTIFY_TEST_2,
        ),
        (
            text!("Test Alert Notification Test 2 Notification Test 2 Notification Test 2"),
            &ALERT_TEST,
        ),
    ] {
        let item = ui.switch_with(label, false, binding)?;
        ui.push(root, item)?;
    }

    let board_settings_title = ui.title(text!("Board Settings"))?;
    ui.push(board_settings, board_settings_title)?;
    let heartbeat_led = ui.switch_with(text!("Heartbeat LED"), true, &NOOP_SWITCH)?;
    ui.push(board_settings, heartbeat_led)?;
    let reverse_keys = ui.switch_with(text!("Reverse Keys"), false, &NOOP_SWITCH)?;
    ui.push(board_settings, reverse_keys)?;
    let display_style = ui.slider_with(text!("Display Style"), 1600, 5, 1..=9999, &NOOP_SLIDER)?;
    ui.push(board_settings, display_style)?;
    let invert_display = ui.switch_with(text!("Invert Display"), false, &NOOP_SWITCH)?;
    ui.push(board_settings, invert_display)?;

    ui.initialize_runtime()?;
    ui.set_startup_logo(startup_logo, Duration::from_millis(500))?;
    ui.initialize_rendering();

    let start_ms = ticks_ms(&mut ui);
    let mut fps_timer = start_ms;
    let mut frame_count = 0;
    let target_ms = 1000.0 / 80.0;

    while !ui.is_closed() {
        let frame_begin = ticks_ms(&mut ui);
        ui.driver_mut().clear();
        ui.render_frame();
        ui.driver_mut().present();

        let mut now_ms = ticks_ms(&mut ui);
        let elapsed_ms = now_ms - frame_begin;
        if elapsed_ms < target_ms {
            ui.driver_mut().delay(Duration::from_millis(
                (target_ms - elapsed_ms).round() as u64
            ));
            now_ms = ticks_ms(&mut ui);
        }

        frame_count += 1;
        if now_ms - fps_timer >= 1000.0 {
            let fps = frame_count as f32 * 1000.0 / (now_ms - fps_timer);
            println!("FPS: {fps:.1}");
            fps_timer = now_ms;
            frame_count = 0;
        }
    }

    Ok(())
}

fn notify_test_1(ui: UiRef, _: bool, _: &'static ()) {
    let _ = ui.notify(text!("Notification Test 1"), Duration::from_millis(5000));
}

fn notify_test_2(ui: UiRef, _: bool, _: &'static ()) {
    let _ = ui.notify(text!("Notification Test 2"), Duration::from_millis(5000));
}

fn alert_test(ui: UiRef, _: bool, _: &'static ()) {
    let _ = ui.alert(text!("Alert Test"), Duration::from_millis(5000));
}

fn noop_switch_changed(_: UiRef, _: bool, _: &'static ()) {}

fn noop_slider_changed(_: UiRef, _: i16, _: &'static ()) {}

fn scene_init(_: UiRef, _: &'static Mutex<SceneState>) {}

fn scene_render(_: UiRef, _: &'static Mutex<SceneState>) {}

fn scene_exit(_: UiRef, state: &'static Mutex<SceneState>) {
    state.lock().unwrap().reset();
}

fn ticks_ms<D>(ui: &mut VisionUi<D>) -> f32
where
    D: vision_ui::driver::Driver + 'static,
{
    ui.driver_mut().ticks().as_secs_f32() * 1000.0
}

fn alloc_with_header(payload_size: usize, zeroed: bool) -> *mut c_void {
    let alloc_size = match size_of::<VisionAllocHeader>().checked_add(payload_size) {
        Some(size) => size,
        None => {
            eprintln!("allocation overflow: size={payload_size}");
            return ptr::null_mut();
        }
    };

    let layout = match Layout::from_size_align(alloc_size, align_of::<VisionAllocHeader>()) {
        Ok(layout) => layout,
        Err(_) => {
            eprintln!("invalid allocation layout: size={alloc_size}");
            return ptr::null_mut();
        }
    };

    let base = unsafe {
        if zeroed {
            alloc_zeroed(layout)
        } else {
            alloc(layout)
        }
    };
    let Some(base) = NonNull::new(base) else {
        if zeroed {
            eprintln!("calloc failed: payload={payload_size}");
        } else {
            eprintln!("malloc failed: payload={payload_size}");
        }
        return ptr::null_mut();
    };

    unsafe {
        base.cast::<VisionAllocHeader>()
            .as_ptr()
            .write(VisionAllocHeader { size: payload_size });
    }

    let total = ALLOC_TOTAL.fetch_add(payload_size, Ordering::SeqCst) + payload_size;
    if zeroed {
        println!("calloc: payload={payload_size}, total={total}");
    } else {
        println!("malloc: payload={payload_size}, total={total}");
    }

    unsafe { base.as_ptr().add(size_of::<VisionAllocHeader>()).cast() }
}

fn free_with_header(ptr: *mut c_void) {
    if ptr.is_null() {
        return;
    }

    let header_ptr = unsafe {
        (ptr.cast::<u8>())
            .sub(size_of::<VisionAllocHeader>())
            .cast::<VisionAllocHeader>()
    };
    let payload_size = unsafe { (*header_ptr).size };
    let alloc_size = match size_of::<VisionAllocHeader>().checked_add(payload_size) {
        Some(size) => size,
        None => {
            eprintln!("free overflow: payload={payload_size}");
            return;
        }
    };
    let layout = match Layout::from_size_align(alloc_size, align_of::<VisionAllocHeader>()) {
        Ok(layout) => layout,
        Err(_) => {
            eprintln!("invalid free layout: payload={payload_size}");
            return;
        }
    };

    let total_before = ALLOC_TOTAL.load(Ordering::SeqCst);
    if payload_size > total_before {
        eprintln!("free detected corrupted size: payload={payload_size}, total={total_before}");
    } else {
        let total = ALLOC_TOTAL.fetch_sub(payload_size, Ordering::SeqCst) - payload_size;
        println!("free: ptr={ptr:p}, payload={payload_size}, total={total}");
    }

    unsafe { dealloc(header_ptr.cast::<u8>(), layout) };
}
