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
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum vision_ui_list_item_type_t {
    ListItem = 0,
    TitleItem = 1,
    IconItem = 2,
    SwitchItem = 3,
    SliderItem = 4,
    UserItem = 5,
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
    pub type_: vision_ui_list_item_type_t,
    pub icon_view_mode: bool,
    pub content: *const c_char,
    pub text_scroll_anchor: u32,
    pub scroll_bar_top: f32,
    pub scroll_bar_top_velocity: f32,
    pub scroll_bar_top_trg: f32,
    pub scroll_bar_height: f32,
    pub scroll_bar_height_velocity: f32,
    pub scroll_bar_height_trg: f32,
    pub scroll_bar_scale_part: f32,
    pub scroll_bar_scale_part_trg: f32,
    pub scroll_bar_top_px: i16,
    pub scroll_bar_height_px: i16,
    pub y_list_item: f32,
    pub y_list_item_trg: f32,
    pub icon_scroll_offset: f32,
    pub icon_scroll_offset_velocity: f32,
    pub icon_scroll_offset_trg: f32,
    pub layer: u8,
    pub child_num: u8,
    pub capacity: usize,
    pub child_list_item: *mut *mut vision_ui_list_item_t,
    pub parent: *mut vision_ui_list_item_t,
}

#[repr(C)]
pub struct vision_ui_switch_item_t {
    pub base_item: vision_ui_list_item_t,
    pub value: bool,
    pub on_changed: Option<extern "C" fn(bool)>,
}

#[repr(C)]
pub struct vision_ui_slider_item_t {
    pub base_item: vision_ui_list_item_t,
    pub value: i16,
    pub is_confirmed: bool,
    pub value_step: u8,
    pub value_max: i16,
    pub value_min: i16,
    pub text_scroll_anchor: u32,
    pub on_changed: Option<extern "C" fn(i16)>,
}

#[repr(C)]
pub struct vision_ui_icon_item_t {
    pub base_item: vision_ui_list_item_t,
    pub title_y: f32,
    pub title_y_velocity: f32,
    pub title_y_trg: f32,
    pub icon: *const u8,
    pub description: *const c_char,
    pub description_scroll_anchor: u32,
}

#[repr(C)]
pub struct vision_ui_user_item_t {
    pub base_item: vision_ui_list_item_t,
    pub in_user_item: bool,
    pub entering_user_item: bool,
    pub exiting_user_item: bool,
    pub init_function: Option<extern "C" fn()>,
    pub loop_function: Option<extern "C" fn()>,
    pub exit_function: Option<extern "C" fn()>,
    pub user_item_inited: bool,
    pub user_item_looping: bool,
}

#[repr(C)]
pub struct vision_ui_notification_t {
    pub content: *const c_char,
    pub span: u16,
    pub y_notification: f32,
    pub y_notification_trg: f32,
    pub w_notification: f32,
    pub w_notification_trg: f32,
    pub is_running: bool,
    pub time_start: u32,
    pub time: u32,
    pub is_dismissing: bool,
    pub dismiss_start: u32,
    pub pending_content: *const c_char,
    pub pending_span: u16,
    pub has_pending: bool,
}

#[repr(C)]
pub struct vision_ui_alert_t {
    pub content: *const c_char,
    pub span: u16,
    pub y_alert: f32,
    pub y_alert_trg: f32,
    pub w_alert: f32,
    pub w_alert_trg: f32,
    pub is_running: bool,
    pub time_start: u32,
    pub time: u32,
}

#[repr(C)]
pub struct vision_ui_selector_t {
    pub y_selector: f32,
    pub y_selector_velocity: f32,
    pub y_selector_trg: f32,
    pub w_selector: f32,
    pub w_selector_velocity: f32,
    pub w_selector_trg: f32,
    pub h_selector: f32,
    pub h_selector_velocity: f32,
    pub h_selector_trg: f32,
    pub selected_index: u8,
    pub selected_item: *mut vision_ui_list_item_t,
    pub scroll_bar_scale_parent: *mut vision_ui_list_item_t,
    pub scroll_bar_scale_part_shared: f32,
    pub has_pending_selection: bool,
    pub pending_selected_index: u8,
    pub pending_selected_item: *mut vision_ui_list_item_t,
}

#[repr(C)]
pub struct vision_ui_camera_t {
    pub x_camera: f32,
    pub x_camera_velocity: f32,
    pub x_camera_trg: f32,
    pub y_camera: f32,
    pub y_camera_velocity: f32,
    pub y_camera_trg: f32,
    pub selector: *mut vision_ui_selector_t,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct vision_ui_icon_t {
    pub list_header: *mut u8,
    pub switch_header: *mut u8,
    pub slider_header: *mut u8,
    pub default_header: *mut u8,
    pub switch_on_footer: *mut u8,
    pub switch_off_footer: *mut u8,
    pub slider_footer: *mut u8,
    pub header_width: usize,
    pub header_height: usize,
    pub footer_width: usize,
    pub footer_height: usize,
}

#[derive(Debug)]
pub enum VisionUiError {
    NulByteInString(NulError),
    BitmapTooLarge(usize),
}

impl From<NulError> for VisionUiError {
    fn from(value: NulError) -> Self { Self::NulByteInString(value) }
}

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
    retained_strings().lock().expect("string mutex poisoned").push(c);
    Ok(ptr)
}

unsafe extern "C" {
    static mut IS_IN_VISION_UI: bool;
    static mut DEFAULT_LIST_ICON: vision_ui_icon_t;

    fn vision_ui_render_init();
    fn vision_ui_core_init();
    fn vision_ui_start_logo_set(bmp: *const u8, span: u32);
    fn vision_ui_step_render();
    fn vision_ui_is_exited() -> bool;
    fn vision_ui_is_background_frozen() -> bool;

    fn vision_ui_driver_action_get() -> vision_ui_action_t;
    fn vision_ui_driver_ticks_ms_get() -> u32;
    fn vision_ui_driver_delay(ms: u32);
    fn vision_ui_driver_bind(driver: *mut c_void);
    fn vision_ui_driver_font_set(font: vision_ui_font_t);
    fn vision_ui_driver_font_get() -> vision_ui_font_t;
    fn vision_ui_driver_str_draw(x: u16, y: u16, str_: *const c_char);
    fn vision_ui_driver_str_utf8_draw(x: u16, y: u16, str_: *const c_char);
    fn vision_ui_driver_str_width_get(str_: *const c_char) -> u16;
    fn vision_ui_driver_str_utf8_width_get(str_: *const c_char) -> u16;
    fn vision_ui_driver_str_height_get() -> u16;
    fn vision_ui_driver_pixel_draw(x: u16, y: u16);
    fn vision_ui_driver_circle_draw(x: u16, y: u16, r: u16);
    fn vision_ui_driver_disc_draw(x: u16, y: u16, r: u16);
    fn vision_ui_driver_box_r_draw(x: u16, y: u16, w: u16, h: u16, r: u16);
    fn vision_ui_driver_box_draw(x: u16, y: u16, w: u16, h: u16);
    fn vision_ui_driver_frame_draw(x: u16, y: u16, w: u16, h: u16);
    fn vision_ui_driver_frame_r_draw(x: u16, y: u16, w: u16, h: u16, r: u16);
    fn vision_ui_driver_line_h_draw(x: u16, y: u16, l: u16);
    fn vision_ui_driver_line_v_draw(x: u16, y: u16, h: u16);
    fn vision_ui_driver_line_draw(x1: u16, y1: u16, x2: u16, y2: u16);
    fn vision_ui_driver_line_h_dotted_draw(x: u16, y: u16, l: u16);
    fn vision_ui_driver_line_v_dotted_draw(x: u16, y: u16, h: u16);
    fn vision_ui_driver_bmp_draw(x: u16, y: u16, w: u16, h: u16, bit_map: *const u8);
    fn vision_ui_driver_color_draw(color: u8);
    fn vision_ui_driver_font_mode_set(mode: u8);
    fn vision_ui_driver_font_direction_set(dir: u8);
    fn vision_ui_driver_clip_window_set(x0: i16, y0: i16, x1: i16, y1: i16);
    fn vision_ui_driver_clip_window_reset();
    fn vision_ui_driver_buffer_clear();
    fn vision_ui_driver_buffer_send();
    fn vision_ui_driver_buffer_area_send(x: u16, y: u16, w: u16, h: u16);
    fn vision_ui_driver_buffer_pointer_get() -> *mut c_void;

    fn vision_ui_smoothstep(t: f32) -> f32;
    fn vision_ui_animation_s_curve(pos: *mut f32, pos_trg: f32, speed: f32, delta_ms: f32);
    fn vision_ui_animation_2nd_ode_no_overshoot(pos: *mut f32, velocity: *mut f32, pos_trg: f32, speed: f32, delta_ms: f32);
    fn vision_ui_animation_2nd_ode_slight_overshoot(pos: *mut f32, velocity: *mut f32, pos_trg: f32, speed: f32, delta_ms: f32);

    fn vision_ui_allocator_set(allocator: Option<extern "C" fn(vision_alloc_op_t, usize, usize, *mut c_void) -> *mut c_void>);
    fn vision_ui_minifont_set(font: vision_ui_font_t);
    fn vision_ui_font_set(font: vision_ui_font_t);
    fn vision_ui_font_set_title(font: vision_ui_font_t);
    fn vision_ui_font_set_subtitle(font: vision_ui_font_t);
    fn vision_ui_minifont_get() -> vision_ui_font_t;
    fn vision_ui_font_get() -> vision_ui_font_t;
    fn vision_ui_font_get_title() -> vision_ui_font_t;
    fn vision_ui_font_get_subtitle() -> vision_ui_font_t;
    fn vision_ui_exit_animation_is_finished() -> bool;
    fn vision_ui_exit_animation_set_is_finished();
    fn vision_ui_exit_animation_start();
    fn vision_ui_enter_animation_is_finished() -> bool;
    fn vision_ui_enter_animation_set_is_finished();
    fn vision_ui_enter_animation_start();
    fn vision_ui_notification_instance_get() -> *const vision_ui_notification_t;
    fn vision_ui_notification_mutable_instance_get() -> *mut vision_ui_notification_t;
    fn vision_ui_notification_push(content: *const c_char, span: u16);
    fn vision_ui_alert_instance_get() -> *const vision_ui_alert_t;
    fn vision_ui_alert_mutable_instance_get() -> *mut vision_ui_alert_t;
    fn vision_ui_alert_push(content: *const c_char, span: u16);
    fn vision_ui_to_list_switch_item(list_item: *mut vision_ui_list_item_t) -> *mut vision_ui_switch_item_t;
    fn vision_ui_to_list_slider_item(list_item: *mut vision_ui_list_item_t) -> *mut vision_ui_slider_item_t;
    fn vision_ui_to_list_icon_item(list_item: *mut vision_ui_list_item_t) -> *mut vision_ui_icon_item_t;
    fn vision_ui_to_list_user_item(list_item: *mut vision_ui_list_item_t) -> *mut vision_ui_user_item_t;
    fn vision_ui_list_item_new(capacity: usize, icon_mode: bool, content: *const c_char) -> *mut vision_ui_list_item_t;
    fn vision_ui_list_title_item_new(title: *const c_char) -> *mut vision_ui_list_item_t;
    fn vision_ui_list_icon_item_new(capacity: usize, icon: *const u8, title: *const c_char, description: *const c_char) -> *mut vision_ui_list_item_t;
    fn vision_ui_list_switch_item_new(content: *const c_char, default_value: bool, on_changed: Option<extern "C" fn(bool)>) -> *mut vision_ui_list_item_t;
    fn vision_ui_list_slider_item_new(content: *const c_char, default_value: i16, step: u8, min: i16, max: i16, on_changed: Option<extern "C" fn(i16)>) -> *mut vision_ui_list_item_t;
    fn vision_ui_list_user_item_new(content: *const c_char, init_function: Option<extern "C" fn()>, loop_function: Option<extern "C" fn()>, exit_function: Option<extern "C" fn()>) -> *mut vision_ui_list_item_t;
    fn vision_ui_root_item_set(item: *mut vision_ui_list_item_t) -> bool;
    fn vision_ui_root_list_get() -> *mut vision_ui_list_item_t;
    fn vision_ui_list_push_item(parent: *mut vision_ui_list_item_t, child: *mut vision_ui_list_item_t) -> bool;
    fn vision_ui_selector_instance_get() -> *const vision_ui_selector_t;
    fn vision_ui_selector_mutable_instance_get() -> *mut vision_ui_selector_t;
    fn vision_ui_selector_t_selector_bind_item(item: *mut vision_ui_list_item_t) -> bool;
    fn vision_ui_selector_go_next_item();
    fn vision_ui_selector_go_prev_item();
    fn vision_ui_selector_jump_to_selected_item();
    fn vision_ui_selector_exit_current_item();
    fn vision_ui_camera_instance_get() -> *const vision_ui_camera_t;
    fn vision_ui_camera_mutable_instance_get() -> *mut vision_ui_camera_t;
    fn vision_ui_camera_instance_x_trg_set(x_trg: f32);
    fn vision_ui_camera_instance_y_trg_set(y_trg: f32);
    fn vision_ui_camera_bind_selector(selector: *mut vision_ui_selector_t);

    fn vision_ui_exit_animation_render(delta_ms: f32);
    fn vision_ui_enter_animation_render(delta_ms: f32);
    fn vision_ui_widget_render();
    fn vision_ui_list_render();
    fn vision_ui_list_icon_set(icon: vision_ui_icon_t);
    fn vision_ui_list_icon_get_current() -> vision_ui_icon_t;
}

pub fn vision_ui_is_in() -> bool { unsafe { IS_IN_VISION_UI } }
pub fn vision_ui_render_init_safe() { unsafe { vision_ui_render_init() } }
pub fn vision_ui_core_init_safe() { unsafe { vision_ui_core_init() } }
pub fn vision_ui_start_logo_set_safe(bitmap: &[u8]) -> Result<(), VisionUiError> {
    let span = u32::try_from(bitmap.len()).map_err(|_| VisionUiError::BitmapTooLarge(bitmap.len()))?;
    let owned = bitmap.to_vec().into_boxed_slice();
    let ptr = owned.as_ptr();
    *retained_logo().lock().expect("logo mutex poisoned") = Some(owned);
    unsafe { vision_ui_start_logo_set(ptr, span) }
    Ok(())
}
pub fn vision_ui_step_render_safe() { unsafe { vision_ui_step_render() } }
pub fn vision_ui_is_exited_safe() -> bool { unsafe { vision_ui_is_exited() } }
pub fn vision_ui_is_background_frozen_safe() -> bool { unsafe { vision_ui_is_background_frozen() } }

pub fn vision_ui_driver_action_get_safe() -> vision_ui_action_t { unsafe { vision_ui_driver_action_get() } }
pub fn vision_ui_driver_ticks_ms_get_safe() -> u32 { unsafe { vision_ui_driver_ticks_ms_get() } }
pub fn vision_ui_driver_delay_safe(ms: u32) { unsafe { vision_ui_driver_delay(ms) } }
pub fn vision_ui_driver_bind_safe(driver: NonNull<c_void>) { unsafe { vision_ui_driver_bind(driver.as_ptr()) } }
pub fn vision_ui_driver_font_set_safe(font: vision_ui_font_t) { unsafe { vision_ui_driver_font_set(font) } }
pub fn vision_ui_driver_font_get_safe() -> vision_ui_font_t { unsafe { vision_ui_driver_font_get() } }
pub fn vision_ui_driver_str_draw_safe(x: u16, y: u16, text: &str) -> Result<(), VisionUiError> { let c=CString::new(text)?; unsafe{vision_ui_driver_str_draw(x,y,c.as_ptr())}; Ok(()) }
pub fn vision_ui_driver_str_utf8_draw_safe(x: u16, y: u16, text: &str) -> Result<(), VisionUiError> { let c=CString::new(text)?; unsafe{vision_ui_driver_str_utf8_draw(x,y,c.as_ptr())}; Ok(()) }
pub fn vision_ui_driver_str_width_get_safe(text: &str) -> Result<u16, VisionUiError> { let c=CString::new(text)?; Ok(unsafe{vision_ui_driver_str_width_get(c.as_ptr())}) }
pub fn vision_ui_driver_str_utf8_width_get_safe(text: &str) -> Result<u16, VisionUiError> { let c=CString::new(text)?; Ok(unsafe{vision_ui_driver_str_utf8_width_get(c.as_ptr())}) }
pub fn vision_ui_driver_str_height_get_safe() -> u16 { unsafe { vision_ui_driver_str_height_get() } }
pub fn vision_ui_driver_pixel_draw_safe(x:u16,y:u16){unsafe{vision_ui_driver_pixel_draw(x,y)}}
pub fn vision_ui_driver_circle_draw_safe(x:u16,y:u16,r:u16){unsafe{vision_ui_driver_circle_draw(x,y,r)}}
pub fn vision_ui_driver_disc_draw_safe(x:u16,y:u16,r:u16){unsafe{vision_ui_driver_disc_draw(x,y,r)}}
pub fn vision_ui_driver_box_r_draw_safe(x:u16,y:u16,w:u16,h:u16,r:u16){unsafe{vision_ui_driver_box_r_draw(x,y,w,h,r)}}
pub fn vision_ui_driver_box_draw_safe(x:u16,y:u16,w:u16,h:u16){unsafe{vision_ui_driver_box_draw(x,y,w,h)}}
pub fn vision_ui_driver_frame_draw_safe(x:u16,y:u16,w:u16,h:u16){unsafe{vision_ui_driver_frame_draw(x,y,w,h)}}
pub fn vision_ui_driver_frame_r_draw_safe(x:u16,y:u16,w:u16,h:u16,r:u16){unsafe{vision_ui_driver_frame_r_draw(x,y,w,h,r)}}
pub fn vision_ui_driver_line_h_draw_safe(x:u16,y:u16,l:u16){unsafe{vision_ui_driver_line_h_draw(x,y,l)}}
pub fn vision_ui_driver_line_v_draw_safe(x:u16,y:u16,h:u16){unsafe{vision_ui_driver_line_v_draw(x,y,h)}}
pub fn vision_ui_driver_line_draw_safe(x1:u16,y1:u16,x2:u16,y2:u16){unsafe{vision_ui_driver_line_draw(x1,y1,x2,y2)}}
pub fn vision_ui_driver_line_h_dotted_draw_safe(x:u16,y:u16,l:u16){unsafe{vision_ui_driver_line_h_dotted_draw(x,y,l)}}
pub fn vision_ui_driver_line_v_dotted_draw_safe(x:u16,y:u16,h:u16){unsafe{vision_ui_driver_line_v_dotted_draw(x,y,h)}}
pub fn vision_ui_driver_bmp_draw_safe(x:u16,y:u16,w:u16,h:u16,bit_map:&[u8]){unsafe{vision_ui_driver_bmp_draw(x,y,w,h,bit_map.as_ptr())}}
pub fn vision_ui_driver_color_draw_safe(color:u8){unsafe{vision_ui_driver_color_draw(color)}}
pub fn vision_ui_driver_font_mode_set_safe(mode:u8){unsafe{vision_ui_driver_font_mode_set(mode)}}
pub fn vision_ui_driver_font_direction_set_safe(dir:u8){unsafe{vision_ui_driver_font_direction_set(dir)}}
pub fn vision_ui_driver_clip_window_set_safe(x0:i16,y0:i16,x1:i16,y1:i16){unsafe{vision_ui_driver_clip_window_set(x0,y0,x1,y1)}}
pub fn vision_ui_driver_clip_window_reset_safe(){unsafe{vision_ui_driver_clip_window_reset()}}
pub fn vision_ui_driver_buffer_clear_safe(){unsafe{vision_ui_driver_buffer_clear()}}
pub fn vision_ui_driver_buffer_send_safe(){unsafe{vision_ui_driver_buffer_send()}}
pub fn vision_ui_driver_buffer_area_send_safe(x:u16,y:u16,w:u16,h:u16){unsafe{vision_ui_driver_buffer_area_send(x,y,w,h)}}
pub fn vision_ui_driver_buffer_pointer_get_safe() -> Option<NonNull<c_void>> { NonNull::new(unsafe { vision_ui_driver_buffer_pointer_get() }) }

pub fn vision_ui_smoothstep_safe(t:f32)->f32{unsafe{vision_ui_smoothstep(t)}}
pub fn vision_ui_animation_s_curve_safe(pos:&mut f32,pos_trg:f32,speed:f32,delta_ms:f32){unsafe{vision_ui_animation_s_curve(pos,pos_trg,speed,delta_ms)}}
pub fn vision_ui_animation_2nd_ode_no_overshoot_safe(pos:&mut f32,velocity:&mut f32,pos_trg:f32,speed:f32,delta_ms:f32){unsafe{vision_ui_animation_2nd_ode_no_overshoot(pos,velocity,pos_trg,speed,delta_ms)}}
pub fn vision_ui_animation_2nd_ode_slight_overshoot_safe(pos:&mut f32,velocity:&mut f32,pos_trg:f32,speed:f32,delta_ms:f32){unsafe{vision_ui_animation_2nd_ode_slight_overshoot(pos,velocity,pos_trg,speed,delta_ms)}}

pub fn vision_ui_allocator_set_safe(allocator: Option<extern "C" fn(vision_alloc_op_t, usize, usize, *mut c_void) -> *mut c_void>) { unsafe { vision_ui_allocator_set(allocator) } }
pub fn vision_ui_minifont_set_safe(font:vision_ui_font_t){unsafe{vision_ui_minifont_set(font)}}
pub fn vision_ui_font_set_safe(font:vision_ui_font_t){unsafe{vision_ui_font_set(font)}}
pub fn vision_ui_font_set_title_safe(font:vision_ui_font_t){unsafe{vision_ui_font_set_title(font)}}
pub fn vision_ui_font_set_subtitle_safe(font:vision_ui_font_t){unsafe{vision_ui_font_set_subtitle(font)}}
pub fn vision_ui_minifont_get_safe()->vision_ui_font_t{unsafe{vision_ui_minifont_get()}}
pub fn vision_ui_font_get_safe()->vision_ui_font_t{unsafe{vision_ui_font_get()}}
pub fn vision_ui_font_get_title_safe()->vision_ui_font_t{unsafe{vision_ui_font_get_title()}}
pub fn vision_ui_font_get_subtitle_safe()->vision_ui_font_t{unsafe{vision_ui_font_get_subtitle()}}
pub fn vision_ui_exit_animation_is_finished_safe()->bool{unsafe{vision_ui_exit_animation_is_finished()}}
pub fn vision_ui_exit_animation_set_is_finished_safe(){unsafe{vision_ui_exit_animation_set_is_finished()}}
pub fn vision_ui_exit_animation_start_safe(){unsafe{vision_ui_exit_animation_start()}}
pub fn vision_ui_enter_animation_is_finished_safe()->bool{unsafe{vision_ui_enter_animation_is_finished()}}
pub fn vision_ui_enter_animation_set_is_finished_safe(){unsafe{vision_ui_enter_animation_set_is_finished()}}
pub fn vision_ui_enter_animation_start_safe(){unsafe{vision_ui_enter_animation_start()}}
pub fn vision_ui_notification_instance_get_safe()->Option<NonNull<vision_ui_notification_t>>{NonNull::new(unsafe{vision_ui_notification_mutable_instance_get()})}
pub fn vision_ui_notification_push_safe(content:&str,span:u16)->Result<(),VisionUiError>{let p=keep_str(content)?; unsafe{vision_ui_notification_push(p,span)}; Ok(())}
pub fn vision_ui_alert_instance_get_safe()->Option<NonNull<vision_ui_alert_t>>{NonNull::new(unsafe{vision_ui_alert_mutable_instance_get()})}
pub fn vision_ui_alert_push_safe(content:&str,span:u16)->Result<(),VisionUiError>{let p=keep_str(content)?; unsafe{vision_ui_alert_push(p,span)}; Ok(())}
pub fn vision_ui_to_list_switch_item_safe(list_item:NonNull<vision_ui_list_item_t>)->Option<NonNull<vision_ui_switch_item_t>>{NonNull::new(unsafe{vision_ui_to_list_switch_item(list_item.as_ptr())})}
pub fn vision_ui_to_list_slider_item_safe(list_item:NonNull<vision_ui_list_item_t>)->Option<NonNull<vision_ui_slider_item_t>>{NonNull::new(unsafe{vision_ui_to_list_slider_item(list_item.as_ptr())})}
pub fn vision_ui_to_list_icon_item_safe(list_item:NonNull<vision_ui_list_item_t>)->Option<NonNull<vision_ui_icon_item_t>>{NonNull::new(unsafe{vision_ui_to_list_icon_item(list_item.as_ptr())})}
pub fn vision_ui_to_list_user_item_safe(list_item:NonNull<vision_ui_list_item_t>)->Option<NonNull<vision_ui_user_item_t>>{NonNull::new(unsafe{vision_ui_to_list_user_item(list_item.as_ptr())})}
pub fn vision_ui_list_item_new_safe(capacity:usize,icon_mode:bool,content:&str)->Result<Option<NonNull<vision_ui_list_item_t>>,VisionUiError>{let p=keep_str(content)?; Ok(NonNull::new(unsafe{vision_ui_list_item_new(capacity,icon_mode,p)}))}
pub fn vision_ui_list_title_item_new_safe(title:&str)->Result<Option<NonNull<vision_ui_list_item_t>>,VisionUiError>{let p=keep_str(title)?; Ok(NonNull::new(unsafe{vision_ui_list_title_item_new(p)}))}
pub fn vision_ui_list_icon_item_new_safe(capacity:usize,icon:Option<&'static [u8]>,title:&str,description:Option<&str>)->Result<Option<NonNull<vision_ui_list_item_t>>,VisionUiError>{let t=keep_str(title)?; let d=match description{Some(v)=>keep_str(v)?,None=>std::ptr::null()}; let ip=icon.map_or(std::ptr::null(),|b|b.as_ptr()); Ok(NonNull::new(unsafe{vision_ui_list_icon_item_new(capacity,ip,t,d)}))}
pub fn vision_ui_list_switch_item_new_safe(content:&str,default_value:bool,on_changed:Option<extern "C" fn(bool)>)->Result<Option<NonNull<vision_ui_list_item_t>>,VisionUiError>{let p=keep_str(content)?; Ok(NonNull::new(unsafe{vision_ui_list_switch_item_new(p,default_value,on_changed)}))}
pub fn vision_ui_list_slider_item_new_safe(content:&str,default_value:i16,step:u8,min:i16,max:i16,on_changed:Option<extern "C" fn(i16)>)->Result<Option<NonNull<vision_ui_list_item_t>>,VisionUiError>{let p=keep_str(content)?; Ok(NonNull::new(unsafe{vision_ui_list_slider_item_new(p,default_value,step,min,max,on_changed)}))}
pub fn vision_ui_list_user_item_new_safe(content:&str,init_function:Option<extern "C" fn()>,loop_function:Option<extern "C" fn()>,exit_function:Option<extern "C" fn()>)->Result<Option<NonNull<vision_ui_list_item_t>>,VisionUiError>{let p=keep_str(content)?; Ok(NonNull::new(unsafe{vision_ui_list_user_item_new(p,init_function,loop_function,exit_function)}))}
pub fn vision_ui_root_item_set_safe(item:NonNull<vision_ui_list_item_t>)->bool{unsafe{vision_ui_root_item_set(item.as_ptr())}}
pub fn vision_ui_root_list_get_safe()->Option<NonNull<vision_ui_list_item_t>>{NonNull::new(unsafe{vision_ui_root_list_get()})}
pub fn vision_ui_list_push_item_safe(parent:NonNull<vision_ui_list_item_t>,child:NonNull<vision_ui_list_item_t>)->bool{unsafe{vision_ui_list_push_item(parent.as_ptr(),child.as_ptr())}}
pub fn vision_ui_selector_instance_get_safe()->Option<NonNull<vision_ui_selector_t>>{NonNull::new(unsafe{vision_ui_selector_mutable_instance_get()})}
pub fn vision_ui_selector_bind_item_safe(item:NonNull<vision_ui_list_item_t>)->bool{unsafe{vision_ui_selector_t_selector_bind_item(item.as_ptr())}}
pub fn vision_ui_selector_go_next_item_safe(){unsafe{vision_ui_selector_go_next_item()}}
pub fn vision_ui_selector_go_prev_item_safe(){unsafe{vision_ui_selector_go_prev_item()}}
pub fn vision_ui_selector_jump_to_selected_item_safe(){unsafe{vision_ui_selector_jump_to_selected_item()}}
pub fn vision_ui_selector_exit_current_item_safe(){unsafe{vision_ui_selector_exit_current_item()}}
pub fn vision_ui_camera_instance_get_safe()->Option<NonNull<vision_ui_camera_t>>{NonNull::new(unsafe{vision_ui_camera_mutable_instance_get()})}
pub fn vision_ui_camera_instance_x_trg_set_safe(x_trg:f32){unsafe{vision_ui_camera_instance_x_trg_set(x_trg)}}
pub fn vision_ui_camera_instance_y_trg_set_safe(y_trg:f32){unsafe{vision_ui_camera_instance_y_trg_set(y_trg)}}
pub fn vision_ui_camera_bind_selector_safe(selector:NonNull<vision_ui_selector_t>){unsafe{vision_ui_camera_bind_selector(selector.as_ptr())}}

pub fn vision_ui_exit_animation_render_safe(delta_ms:f32){unsafe{vision_ui_exit_animation_render(delta_ms)}}
pub fn vision_ui_enter_animation_render_safe(delta_ms:f32){unsafe{vision_ui_enter_animation_render(delta_ms)}}
pub fn vision_ui_widget_render_safe(){unsafe{vision_ui_widget_render()}}
pub fn vision_ui_list_render_safe(){unsafe{vision_ui_list_render()}}
pub fn vision_ui_default_list_icon_get_safe()->vision_ui_icon_t{unsafe{DEFAULT_LIST_ICON}}
pub fn vision_ui_list_icon_set_safe(icon:vision_ui_icon_t){unsafe{vision_ui_list_icon_set(icon)}}
pub fn vision_ui_list_icon_get_current_safe()->vision_ui_icon_t{unsafe{vision_ui_list_icon_get_current()}}

pub fn vision_ui_notification_instance_readonly_safe() -> Option<NonNull<vision_ui_notification_t>> {
    NonNull::new(unsafe { vision_ui_notification_instance_get() as *mut vision_ui_notification_t })
}
pub fn vision_ui_alert_instance_readonly_safe() -> Option<NonNull<vision_ui_alert_t>> {
    NonNull::new(unsafe { vision_ui_alert_instance_get() as *mut vision_ui_alert_t })
}
pub fn vision_ui_selector_instance_readonly_safe() -> Option<NonNull<vision_ui_selector_t>> {
    NonNull::new(unsafe { vision_ui_selector_instance_get() as *mut vision_ui_selector_t })
}
pub fn vision_ui_camera_instance_readonly_safe() -> Option<NonNull<vision_ui_camera_t>> {
    NonNull::new(unsafe { vision_ui_camera_instance_get() as *mut vision_ui_camera_t })
}
