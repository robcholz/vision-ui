use std::ffi::{c_void, CString, NulError};
use std::os::raw::c_char;
use std::ptr::NonNull;
use std::sync::{Mutex, OnceLock};

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct vision_ui_font_t {
    pub font: *const c_void,
    pub top_compensation: i8,
    pub bottom_compensation: i8,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum vision_ui_action_t {
    UiActionNone = 0,
    UiActionGoPrev = 1,
    UiActionGoNext = 2,
    UiActionEnter = 3,
    UiActionExit = 4,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub enum vision_alloc_op_t {
    VisionAllocMalloc = 0,
    VisionAllocCalloc = 1,
    VisionAllocFree = 2,
}

#[repr(C)]
pub struct vision_ui_list_item_t {
    _private: [u8; 0],
}

#[repr(C)]
pub struct vision_ui_switch_item_t {
    _private: [u8; 0],
}

#[repr(C)]
pub struct vision_ui_slider_item_t {
    _private: [u8; 0],
}

#[repr(C)]
pub struct vision_ui_icon_item_t {
    _private: [u8; 0],
}

#[repr(C)]
pub struct vision_ui_user_item_t {
    _private: [u8; 0],
}

#[repr(C)]
pub struct vision_ui_notification_t {
    _private: [u8; 0],
}

#[repr(C)]
pub struct vision_ui_alert_t {
    _private: [u8; 0],
}

#[derive(Debug)]
pub enum VisionUiError {
    NulByteInString(NulError),
    BitmapTooLarge(usize),
}

impl From<NulError> for VisionUiError {
    fn from(value: NulError) -> Self {
        Self::NulByteInString(value)
    }
}

impl std::fmt::Display for VisionUiError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::NulByteInString(err) => write!(f, "invalid string contains NUL: {err}"),
            Self::BitmapTooLarge(size) => write!(f, "bitmap length {size} exceeds u32::MAX"),
        }
    }
}

impl std::error::Error for VisionUiError {}

fn retained_strings() -> &'static Mutex<Vec<CString>> {
    static RETAINED_STRINGS: OnceLock<Mutex<Vec<CString>>> = OnceLock::new();
    RETAINED_STRINGS.get_or_init(|| Mutex::new(Vec::new()))
}

fn retained_logo() -> &'static Mutex<Option<Box<[u8]>>> {
    static RETAINED_LOGO: OnceLock<Mutex<Option<Box<[u8]>>>> = OnceLock::new();
    RETAINED_LOGO.get_or_init(|| Mutex::new(None))
}

fn keep_str(text: &str) -> Result<*const c_char, VisionUiError> {
    let c = CString::new(text)?;
    let ptr = c.as_ptr();
    retained_strings()
        .lock()
        .expect("string mutex poisoned")
        .push(c);
    Ok(ptr)
}

unsafe extern "C" {
    static mut IS_IN_VISION_UI: bool;

    #[link_name = "vision_ui_render_init"]
    fn c_vision_ui_render_init();
    #[link_name = "vision_ui_core_init"]
    fn c_vision_ui_core_init();
    #[link_name = "vision_ui_start_logo_set"]
    fn c_vision_ui_start_logo_set(bmp: *const u8, span: u32);
    #[link_name = "vision_ui_step_render"]
    fn c_vision_ui_step_render();
    #[link_name = "vision_ui_is_exited"]
    fn c_vision_ui_is_exited() -> bool;
    #[link_name = "vision_ui_is_background_frozen"]
    fn c_vision_ui_is_background_frozen() -> bool;

    #[link_name = "vision_ui_allocator_set"]
    fn c_vision_ui_allocator_set(
        allocator: Option<extern "C" fn(vision_alloc_op_t, usize, usize, *mut c_void) -> *mut c_void>,
    );
    #[link_name = "vision_ui_minifont_set"]
    fn c_vision_ui_minifont_set(font: vision_ui_font_t);
    #[link_name = "vision_ui_font_set"]
    fn c_vision_ui_font_set(font: vision_ui_font_t);
    #[link_name = "vision_ui_font_set_title"]
    fn c_vision_ui_font_set_title(font: vision_ui_font_t);
    #[link_name = "vision_ui_font_set_subtitle"]
    fn c_vision_ui_font_set_subtitle(font: vision_ui_font_t);
    #[link_name = "vision_ui_minifont_get"]
    fn c_vision_ui_minifont_get() -> vision_ui_font_t;
    #[link_name = "vision_ui_font_get"]
    fn c_vision_ui_font_get() -> vision_ui_font_t;
    #[link_name = "vision_ui_font_get_title"]
    fn c_vision_ui_font_get_title() -> vision_ui_font_t;
    #[link_name = "vision_ui_font_get_subtitle"]
    fn c_vision_ui_font_get_subtitle() -> vision_ui_font_t;

    #[link_name = "vision_ui_notification_instance_get"]
    fn c_vision_ui_notification_instance_get() -> *const vision_ui_notification_t;
    #[link_name = "vision_ui_notification_mutable_instance_get"]
    fn c_vision_ui_notification_mutable_instance_get() -> *mut vision_ui_notification_t;
    #[link_name = "vision_ui_notification_push"]
    fn c_vision_ui_notification_push(content: *const c_char, span: u16);
    #[link_name = "vision_ui_alert_instance_get"]
    fn c_vision_ui_alert_instance_get() -> *const vision_ui_alert_t;
    #[link_name = "vision_ui_alert_mutable_instance_get"]
    fn c_vision_ui_alert_mutable_instance_get() -> *mut vision_ui_alert_t;
    #[link_name = "vision_ui_alert_push"]
    fn c_vision_ui_alert_push(content: *const c_char, span: u16);

    #[link_name = "vision_ui_to_list_switch_item"]
    fn c_vision_ui_to_list_switch_item(list_item: *mut vision_ui_list_item_t) -> *mut vision_ui_switch_item_t;
    #[link_name = "vision_ui_to_list_slider_item"]
    fn c_vision_ui_to_list_slider_item(list_item: *mut vision_ui_list_item_t) -> *mut vision_ui_slider_item_t;
    #[link_name = "vision_ui_to_list_icon_item"]
    fn c_vision_ui_to_list_icon_item(list_item: *mut vision_ui_list_item_t) -> *mut vision_ui_icon_item_t;
    #[link_name = "vision_ui_to_list_user_item"]
    fn c_vision_ui_to_list_user_item(list_item: *mut vision_ui_list_item_t) -> *mut vision_ui_user_item_t;
    #[link_name = "vision_ui_list_item_new"]
    fn c_vision_ui_list_item_new(capacity: usize, icon_mode: bool, content: *const c_char) -> *mut vision_ui_list_item_t;
    #[link_name = "vision_ui_list_title_item_new"]
    fn c_vision_ui_list_title_item_new(title: *const c_char) -> *mut vision_ui_list_item_t;
    #[link_name = "vision_ui_list_icon_item_new"]
    fn c_vision_ui_list_icon_item_new(
        capacity: usize,
        icon: *const u8,
        title: *const c_char,
        description: *const c_char,
    ) -> *mut vision_ui_list_item_t;
    #[link_name = "vision_ui_list_switch_item_new"]
    fn c_vision_ui_list_switch_item_new(
        content: *const c_char,
        default_value: bool,
        on_changed: Option<extern "C" fn(bool)>,
    ) -> *mut vision_ui_list_item_t;
    #[link_name = "vision_ui_list_slider_item_new"]
    fn c_vision_ui_list_slider_item_new(
        content: *const c_char,
        default_value: i16,
        step: u8,
        min: i16,
        max: i16,
        on_changed: Option<extern "C" fn(i16)>,
    ) -> *mut vision_ui_list_item_t;
    #[link_name = "vision_ui_list_user_item_new"]
    fn c_vision_ui_list_user_item_new(
        content: *const c_char,
        init_function: Option<extern "C" fn()>,
        loop_function: Option<extern "C" fn()>,
        exit_function: Option<extern "C" fn()>,
    ) -> *mut vision_ui_list_item_t;
    #[link_name = "vision_ui_root_item_set"]
    fn c_vision_ui_root_item_set(item: *mut vision_ui_list_item_t) -> bool;
    #[link_name = "vision_ui_root_list_get"]
    fn c_vision_ui_root_list_get() -> *mut vision_ui_list_item_t;
    #[link_name = "vision_ui_list_push_item"]
    fn c_vision_ui_list_push_item(parent: *mut vision_ui_list_item_t, child: *mut vision_ui_list_item_t) -> bool;

    #[link_name = "vision_ui_driver_action_get"]
    fn c_vision_ui_driver_action_get() -> vision_ui_action_t;
    #[link_name = "vision_ui_driver_ticks_ms_get"]
    fn c_vision_ui_driver_ticks_ms_get() -> u32;
    #[link_name = "vision_ui_driver_delay"]
    fn c_vision_ui_driver_delay(ms: u32);
    #[link_name = "vision_ui_driver_bind"]
    fn c_vision_ui_driver_bind(driver: *mut c_void);
    #[link_name = "vision_ui_driver_font_set"]
    fn c_vision_ui_driver_font_set(font: vision_ui_font_t);
    #[link_name = "vision_ui_driver_font_get"]
    fn c_vision_ui_driver_font_get() -> vision_ui_font_t;
    #[link_name = "vision_ui_driver_str_draw"]
    fn c_vision_ui_driver_str_draw(x: u16, y: u16, str_: *const c_char);
    #[link_name = "vision_ui_driver_str_utf8_draw"]
    fn c_vision_ui_driver_str_utf8_draw(x: u16, y: u16, str_: *const c_char);
    #[link_name = "vision_ui_driver_str_width_get"]
    fn c_vision_ui_driver_str_width_get(str_: *const c_char) -> u16;
    #[link_name = "vision_ui_driver_str_utf8_width_get"]
    fn c_vision_ui_driver_str_utf8_width_get(str_: *const c_char) -> u16;
    #[link_name = "vision_ui_driver_str_height_get"]
    fn c_vision_ui_driver_str_height_get() -> u16;
    #[link_name = "vision_ui_driver_pixel_draw"]
    fn c_vision_ui_driver_pixel_draw(x: u16, y: u16);
    #[link_name = "vision_ui_driver_circle_draw"]
    fn c_vision_ui_driver_circle_draw(x: u16, y: u16, r: u16);
    #[link_name = "vision_ui_driver_disc_draw"]
    fn c_vision_ui_driver_disc_draw(x: u16, y: u16, r: u16);
    #[link_name = "vision_ui_driver_box_r_draw"]
    fn c_vision_ui_driver_box_r_draw(x: u16, y: u16, w: u16, h: u16, r: u16);
    #[link_name = "vision_ui_driver_box_draw"]
    fn c_vision_ui_driver_box_draw(x: u16, y: u16, w: u16, h: u16);
    #[link_name = "vision_ui_driver_frame_draw"]
    fn c_vision_ui_driver_frame_draw(x: u16, y: u16, w: u16, h: u16);
    #[link_name = "vision_ui_driver_frame_r_draw"]
    fn c_vision_ui_driver_frame_r_draw(x: u16, y: u16, w: u16, h: u16, r: u16);
    #[link_name = "vision_ui_driver_line_h_draw"]
    fn c_vision_ui_driver_line_h_draw(x: u16, y: u16, l: u16);
    #[link_name = "vision_ui_driver_line_v_draw"]
    fn c_vision_ui_driver_line_v_draw(x: u16, y: u16, h: u16);
    #[link_name = "vision_ui_driver_line_draw"]
    fn c_vision_ui_driver_line_draw(x1: u16, y1: u16, x2: u16, y2: u16);
    #[link_name = "vision_ui_driver_line_h_dotted_draw"]
    fn c_vision_ui_driver_line_h_dotted_draw(x: u16, y: u16, l: u16);
    #[link_name = "vision_ui_driver_line_v_dotted_draw"]
    fn c_vision_ui_driver_line_v_dotted_draw(x: u16, y: u16, h: u16);
    #[link_name = "vision_ui_driver_bmp_draw"]
    fn c_vision_ui_driver_bmp_draw(x: u16, y: u16, w: u16, h: u16, bit_map: *const u8);
    #[link_name = "vision_ui_driver_color_draw"]
    fn c_vision_ui_driver_color_draw(color: u8);
    #[link_name = "vision_ui_driver_font_mode_set"]
    fn c_vision_ui_driver_font_mode_set(mode: u8);
    #[link_name = "vision_ui_driver_font_direction_set"]
    fn c_vision_ui_driver_font_direction_set(dir: u8);
    #[link_name = "vision_ui_driver_clip_window_set"]
    fn c_vision_ui_driver_clip_window_set(x0: i16, y0: i16, x1: i16, y1: i16);
    #[link_name = "vision_ui_driver_clip_window_reset"]
    fn c_vision_ui_driver_clip_window_reset();
    #[link_name = "vision_ui_driver_buffer_clear"]
    fn c_vision_ui_driver_buffer_clear();
    #[link_name = "vision_ui_driver_buffer_send"]
    fn c_vision_ui_driver_buffer_send();
    #[link_name = "vision_ui_driver_buffer_area_send"]
    fn c_vision_ui_driver_buffer_area_send(x: u16, y: u16, w: u16, h: u16);
    #[link_name = "vision_ui_driver_buffer_pointer_get"]
    fn c_vision_ui_driver_buffer_pointer_get() -> *mut c_void;
}

pub fn is_in_vision_ui() -> bool {
    unsafe { IS_IN_VISION_UI }
}

pub fn vision_ui_render_init() {
    unsafe { c_vision_ui_render_init() }
}
pub fn vision_ui_core_init() {
    unsafe { c_vision_ui_core_init() }
}
pub fn vision_ui_start_logo_set(bitmap: &[u8]) -> Result<(), VisionUiError> {
    let span = u32::try_from(bitmap.len()).map_err(|_| VisionUiError::BitmapTooLarge(bitmap.len()))?;
    let owned = bitmap.to_vec().into_boxed_slice();
    let ptr = owned.as_ptr();
    *retained_logo().lock().expect("logo mutex poisoned") = Some(owned);
    unsafe { c_vision_ui_start_logo_set(ptr, span) }
    Ok(())
}
pub fn vision_ui_step_render() {
    unsafe { c_vision_ui_step_render() }
}
pub fn vision_ui_is_exited() -> bool {
    unsafe { c_vision_ui_is_exited() }
}
pub fn vision_ui_is_background_frozen() -> bool {
    unsafe { c_vision_ui_is_background_frozen() }
}

pub fn vision_ui_allocator_set(
    allocator: Option<extern "C" fn(vision_alloc_op_t, usize, usize, *mut c_void) -> *mut c_void>,
) {
    unsafe { c_vision_ui_allocator_set(allocator) }
}
pub fn vision_ui_minifont_set(font: vision_ui_font_t) {
    unsafe { c_vision_ui_minifont_set(font) }
}
pub fn vision_ui_font_set(font: vision_ui_font_t) {
    unsafe { c_vision_ui_font_set(font) }
}
pub fn vision_ui_font_set_title(font: vision_ui_font_t) {
    unsafe { c_vision_ui_font_set_title(font) }
}
pub fn vision_ui_font_set_subtitle(font: vision_ui_font_t) {
    unsafe { c_vision_ui_font_set_subtitle(font) }
}
pub fn vision_ui_minifont_get() -> vision_ui_font_t {
    unsafe { c_vision_ui_minifont_get() }
}
pub fn vision_ui_font_get() -> vision_ui_font_t {
    unsafe { c_vision_ui_font_get() }
}
pub fn vision_ui_font_get_title() -> vision_ui_font_t {
    unsafe { c_vision_ui_font_get_title() }
}
pub fn vision_ui_font_get_subtitle() -> vision_ui_font_t {
    unsafe { c_vision_ui_font_get_subtitle() }
}

pub fn vision_ui_notification_instance_get() -> Option<NonNull<vision_ui_notification_t>> {
    NonNull::new(unsafe { c_vision_ui_notification_instance_get() as *mut vision_ui_notification_t })
}
pub fn vision_ui_notification_mutable_instance_get() -> Option<NonNull<vision_ui_notification_t>> {
    NonNull::new(unsafe { c_vision_ui_notification_mutable_instance_get() })
}
pub fn vision_ui_notification_push(content: &str, span: u16) -> Result<(), VisionUiError> {
    let p = keep_str(content)?;
    unsafe { c_vision_ui_notification_push(p, span) }
    Ok(())
}
pub fn vision_ui_alert_instance_get() -> Option<NonNull<vision_ui_alert_t>> {
    NonNull::new(unsafe { c_vision_ui_alert_instance_get() as *mut vision_ui_alert_t })
}
pub fn vision_ui_alert_mutable_instance_get() -> Option<NonNull<vision_ui_alert_t>> {
    NonNull::new(unsafe { c_vision_ui_alert_mutable_instance_get() })
}
pub fn vision_ui_alert_push(content: &str, span: u16) -> Result<(), VisionUiError> {
    let p = keep_str(content)?;
    unsafe { c_vision_ui_alert_push(p, span) }
    Ok(())
}

pub fn vision_ui_to_list_switch_item(
    list_item: NonNull<vision_ui_list_item_t>,
) -> Option<NonNull<vision_ui_switch_item_t>> {
    NonNull::new(unsafe { c_vision_ui_to_list_switch_item(list_item.as_ptr()) })
}
pub fn vision_ui_to_list_slider_item(
    list_item: NonNull<vision_ui_list_item_t>,
) -> Option<NonNull<vision_ui_slider_item_t>> {
    NonNull::new(unsafe { c_vision_ui_to_list_slider_item(list_item.as_ptr()) })
}
pub fn vision_ui_to_list_icon_item(
    list_item: NonNull<vision_ui_list_item_t>,
) -> Option<NonNull<vision_ui_icon_item_t>> {
    NonNull::new(unsafe { c_vision_ui_to_list_icon_item(list_item.as_ptr()) })
}
pub fn vision_ui_to_list_user_item(
    list_item: NonNull<vision_ui_list_item_t>,
) -> Option<NonNull<vision_ui_user_item_t>> {
    NonNull::new(unsafe { c_vision_ui_to_list_user_item(list_item.as_ptr()) })
}

pub fn vision_ui_list_item_new(
    capacity: usize,
    icon_mode: bool,
    content: &str,
) -> Result<Option<NonNull<vision_ui_list_item_t>>, VisionUiError> {
    let p = keep_str(content)?;
    Ok(NonNull::new(unsafe { c_vision_ui_list_item_new(capacity, icon_mode, p) }))
}
pub fn vision_ui_list_title_item_new(
    title: &str,
) -> Result<Option<NonNull<vision_ui_list_item_t>>, VisionUiError> {
    let p = keep_str(title)?;
    Ok(NonNull::new(unsafe { c_vision_ui_list_title_item_new(p) }))
}
pub fn vision_ui_list_icon_item_new(
    capacity: usize,
    icon: Option<&'static [u8]>,
    title: &str,
    description: Option<&str>,
) -> Result<Option<NonNull<vision_ui_list_item_t>>, VisionUiError> {
    let t = keep_str(title)?;
    let d = match description {
        Some(v) => keep_str(v)?,
        None => std::ptr::null(),
    };
    let ip = icon.map_or(std::ptr::null(), |b| b.as_ptr());
    Ok(NonNull::new(unsafe { c_vision_ui_list_icon_item_new(capacity, ip, t, d) }))
}
pub fn vision_ui_list_switch_item_new(
    content: &str,
    default_value: bool,
    on_changed: Option<extern "C" fn(bool)>,
) -> Result<Option<NonNull<vision_ui_list_item_t>>, VisionUiError> {
    let p = keep_str(content)?;
    Ok(NonNull::new(unsafe {
        c_vision_ui_list_switch_item_new(p, default_value, on_changed)
    }))
}
pub fn vision_ui_list_slider_item_new(
    content: &str,
    default_value: i16,
    step: u8,
    min: i16,
    max: i16,
    on_changed: Option<extern "C" fn(i16)>,
) -> Result<Option<NonNull<vision_ui_list_item_t>>, VisionUiError> {
    let p = keep_str(content)?;
    Ok(NonNull::new(unsafe {
        c_vision_ui_list_slider_item_new(p, default_value, step, min, max, on_changed)
    }))
}
pub fn vision_ui_list_user_item_new(
    content: &str,
    init_function: Option<extern "C" fn()>,
    loop_function: Option<extern "C" fn()>,
    exit_function: Option<extern "C" fn()>,
) -> Result<Option<NonNull<vision_ui_list_item_t>>, VisionUiError> {
    let p = keep_str(content)?;
    Ok(NonNull::new(unsafe {
        c_vision_ui_list_user_item_new(p, init_function, loop_function, exit_function)
    }))
}
pub fn vision_ui_root_item_set(item: NonNull<vision_ui_list_item_t>) -> bool {
    unsafe { c_vision_ui_root_item_set(item.as_ptr()) }
}
pub fn vision_ui_root_list_get() -> Option<NonNull<vision_ui_list_item_t>> {
    NonNull::new(unsafe { c_vision_ui_root_list_get() })
}
pub fn vision_ui_list_push_item(
    parent: NonNull<vision_ui_list_item_t>,
    child: NonNull<vision_ui_list_item_t>,
) -> bool {
    unsafe { c_vision_ui_list_push_item(parent.as_ptr(), child.as_ptr()) }
}

pub fn vision_ui_driver_action_get() -> vision_ui_action_t {
    unsafe { c_vision_ui_driver_action_get() }
}
pub fn vision_ui_driver_ticks_ms_get() -> u32 {
    unsafe { c_vision_ui_driver_ticks_ms_get() }
}
pub fn vision_ui_driver_delay(ms: u32) {
    unsafe { c_vision_ui_driver_delay(ms) }
}
pub fn vision_ui_driver_bind(driver: NonNull<c_void>) {
    unsafe { c_vision_ui_driver_bind(driver.as_ptr()) }
}
pub fn vision_ui_driver_font_set(font: vision_ui_font_t) {
    unsafe { c_vision_ui_driver_font_set(font) }
}
pub fn vision_ui_driver_font_get() -> vision_ui_font_t {
    unsafe { c_vision_ui_driver_font_get() }
}
pub fn vision_ui_driver_str_draw(x: u16, y: u16, text: &str) -> Result<(), VisionUiError> {
    let c = CString::new(text)?;
    unsafe { c_vision_ui_driver_str_draw(x, y, c.as_ptr()) }
    Ok(())
}
pub fn vision_ui_driver_str_utf8_draw(x: u16, y: u16, text: &str) -> Result<(), VisionUiError> {
    let c = CString::new(text)?;
    unsafe { c_vision_ui_driver_str_utf8_draw(x, y, c.as_ptr()) }
    Ok(())
}
pub fn vision_ui_driver_str_width_get(text: &str) -> Result<u16, VisionUiError> {
    let c = CString::new(text)?;
    Ok(unsafe { c_vision_ui_driver_str_width_get(c.as_ptr()) })
}
pub fn vision_ui_driver_str_utf8_width_get(text: &str) -> Result<u16, VisionUiError> {
    let c = CString::new(text)?;
    Ok(unsafe { c_vision_ui_driver_str_utf8_width_get(c.as_ptr()) })
}
pub fn vision_ui_driver_str_height_get() -> u16 {
    unsafe { c_vision_ui_driver_str_height_get() }
}
pub fn vision_ui_driver_pixel_draw(x: u16, y: u16) {
    unsafe { c_vision_ui_driver_pixel_draw(x, y) }
}
pub fn vision_ui_driver_circle_draw(x: u16, y: u16, r: u16) {
    unsafe { c_vision_ui_driver_circle_draw(x, y, r) }
}
pub fn vision_ui_driver_disc_draw(x: u16, y: u16, r: u16) {
    unsafe { c_vision_ui_driver_disc_draw(x, y, r) }
}
pub fn vision_ui_driver_box_r_draw(x: u16, y: u16, w: u16, h: u16, r: u16) {
    unsafe { c_vision_ui_driver_box_r_draw(x, y, w, h, r) }
}
pub fn vision_ui_driver_box_draw(x: u16, y: u16, w: u16, h: u16) {
    unsafe { c_vision_ui_driver_box_draw(x, y, w, h) }
}
pub fn vision_ui_driver_frame_draw(x: u16, y: u16, w: u16, h: u16) {
    unsafe { c_vision_ui_driver_frame_draw(x, y, w, h) }
}
pub fn vision_ui_driver_frame_r_draw(x: u16, y: u16, w: u16, h: u16, r: u16) {
    unsafe { c_vision_ui_driver_frame_r_draw(x, y, w, h, r) }
}
pub fn vision_ui_driver_line_h_draw(x: u16, y: u16, l: u16) {
    unsafe { c_vision_ui_driver_line_h_draw(x, y, l) }
}
pub fn vision_ui_driver_line_v_draw(x: u16, y: u16, h: u16) {
    unsafe { c_vision_ui_driver_line_v_draw(x, y, h) }
}
pub fn vision_ui_driver_line_draw(x1: u16, y1: u16, x2: u16, y2: u16) {
    unsafe { c_vision_ui_driver_line_draw(x1, y1, x2, y2) }
}
pub fn vision_ui_driver_line_h_dotted_draw(x: u16, y: u16, l: u16) {
    unsafe { c_vision_ui_driver_line_h_dotted_draw(x, y, l) }
}
pub fn vision_ui_driver_line_v_dotted_draw(x: u16, y: u16, h: u16) {
    unsafe { c_vision_ui_driver_line_v_dotted_draw(x, y, h) }
}
pub fn vision_ui_driver_bmp_draw(x: u16, y: u16, w: u16, h: u16, bit_map: &[u8]) {
    unsafe { c_vision_ui_driver_bmp_draw(x, y, w, h, bit_map.as_ptr()) }
}
pub fn vision_ui_driver_color_draw(color: u8) {
    unsafe { c_vision_ui_driver_color_draw(color) }
}
pub fn vision_ui_driver_font_mode_set(mode: u8) {
    unsafe { c_vision_ui_driver_font_mode_set(mode) }
}
pub fn vision_ui_driver_font_direction_set(dir: u8) {
    unsafe { c_vision_ui_driver_font_direction_set(dir) }
}
pub fn vision_ui_driver_clip_window_set(x0: i16, y0: i16, x1: i16, y1: i16) {
    unsafe { c_vision_ui_driver_clip_window_set(x0, y0, x1, y1) }
}
pub fn vision_ui_driver_clip_window_reset() {
    unsafe { c_vision_ui_driver_clip_window_reset() }
}
pub fn vision_ui_driver_buffer_clear() {
    unsafe { c_vision_ui_driver_buffer_clear() }
}
pub fn vision_ui_driver_buffer_send() {
    unsafe { c_vision_ui_driver_buffer_send() }
}
pub fn vision_ui_driver_buffer_area_send(x: u16, y: u16, w: u16, h: u16) {
    unsafe { c_vision_ui_driver_buffer_area_send(x, y, w, h) }
}
pub fn vision_ui_driver_buffer_pointer_get() -> Option<NonNull<c_void>> {
    NonNull::new(unsafe { c_vision_ui_driver_buffer_pointer_get() })
}
