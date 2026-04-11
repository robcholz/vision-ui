use std::cell::RefCell;
use std::ffi::{c_void, CString, NulError};
use std::ptr::NonNull;
use std::rc::{Rc, Weak};
use std::time::Duration;
use thiserror::Error;

#[path = "config_bindings.rs"]
mod config;
pub mod driver;
#[path = "bindings.rs"]
pub mod raw;

pub use raw::vision_ui_font_t as Font;
pub use raw::vision_ui_icon_t as IconPack;

pub const SCREEN_WIDTH: u16 = config::VISION_UI_SCREEN_WIDTH as u16;
pub const SCREEN_HEIGHT: u16 = config::VISION_UI_SCREEN_HEIGHT as u16;
pub const ICON_SIZE: u16 = config::VISION_UI_ICON_VIEW_ICON_SIZE as u16;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Action {
    None,
    Previous,
    Next,
    Enter,
    Exit,
    Unknown(u32),
}

impl From<raw::vision_ui_action_t> for Action {
    fn from(value: raw::vision_ui_action_t) -> Self {
        match value {
            raw::vision_ui_action_t_UiActionNone => Self::None,
            raw::vision_ui_action_t_UiActionGoPrev => Self::Previous,
            raw::vision_ui_action_t_UiActionGoNext => Self::Next,
            raw::vision_ui_action_t_UiActionEnter => Self::Enter,
            raw::vision_ui_action_t_UiActionExit => Self::Exit,
            other => Self::Unknown(other),
        }
    }
}

#[derive(Debug, Error)]
pub enum Error {
    #[error("allocation failed for {0}")]
    AllocationFailed(&'static str),
    #[error("string contains interior NUL: {0}")]
    InteriorNul(NulError),
    #[error("bitmap length {0} exceeds u32::MAX")]
    BitmapTooLarge(usize),
    #[error("bitmap length {actual} does not match {width}x{height} XBM payload size {expected}")]
    InvalidBitmapLength {
        width: u16,
        height: u16,
        expected: usize,
        actual: usize,
    },
    #[error("bitmap dimensions {actual_width}x{actual_height} do not match expected {expected_width}x{expected_height}")]
    BitmapDimensionsMismatch {
        expected_width: u16,
        expected_height: u16,
        actual_width: u16,
        actual_height: u16,
    },
    #[error("duration {0:?} exceeds Vision UI's integer range")]
    DurationOverflow(Duration),
}

impl From<NulError> for Error {
    fn from(value: NulError) -> Self {
        Self::InteriorNul(value)
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Item(NonNull<raw::vision_ui_list_item_t>);

impl Item {
    pub fn as_ptr(self) -> *mut raw::vision_ui_list_item_t {
        self.0.as_ptr()
    }

    pub fn as_non_null(self) -> NonNull<raw::vision_ui_list_item_t> {
        self.0
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct MonoBitmap<'a> {
    bytes: &'a [u8],
    width: u16,
    height: u16,
}

impl<'a> MonoBitmap<'a> {
    pub fn new(bytes: &'a [u8], width: u16, height: u16) -> Result<Self, Error> {
        let expected = bitmap_len(width, height);
        if bytes.len() != expected {
            return Err(Error::InvalidBitmapLength {
                width,
                height,
                expected,
                actual: bytes.len(),
            });
        }
        Ok(Self {
            bytes,
            width,
            height,
        })
    }

    pub fn bytes(self) -> &'a [u8] {
        self.bytes
    }

    pub fn width(self) -> u16 {
        self.width
    }

    pub fn height(self) -> u16 {
        self.height
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct StartupLogo<'a>(MonoBitmap<'a>);

impl<'a> StartupLogo<'a> {
    pub fn new(bytes: &'a [u8]) -> Result<Self, Error> {
        Ok(Self(MonoBitmap::new(bytes, SCREEN_WIDTH, SCREEN_HEIGHT)?))
    }

    pub fn bitmap(self) -> MonoBitmap<'a> {
        self.0
    }
}

type ToggleFn = dyn FnMut(UiRef, bool);
type SlideFn = dyn FnMut(UiRef, i16);
type SceneFn = dyn FnMut(UiRef);

struct ToggleCallback {
    state: Weak<RefCell<State>>,
    handler: Box<ToggleFn>,
}

struct SlideCallback {
    state: Weak<RefCell<State>>,
    handler: Box<SlideFn>,
}

struct SceneCallbackSet {
    state: Weak<RefCell<State>>,
    init: Option<Box<SceneFn>>,
    render: Option<Box<SceneFn>>,
    exit: Option<Box<SceneFn>>,
}

struct State {
    raw: NonNull<raw::vision_ui_t>,
    retained_strings: Vec<CString>,
    retained_bitmaps: Vec<Box<[u8]>>,
    toggle_callbacks: Vec<Box<ToggleCallback>>,
    slide_callbacks: Vec<Box<SlideCallback>>,
    scene_callbacks: Vec<Box<SceneCallbackSet>>,
}

impl State {
    fn retain_c_string(&mut self, value: &str) -> Result<*const std::os::raw::c_char, Error> {
        let string = CString::new(value)?;
        let ptr = string.as_ptr();
        self.retained_strings.push(string);
        Ok(ptr)
    }

    fn retain_bitmap(&mut self, bitmap: &[u8]) -> &Box<[u8]> {
        self.retained_bitmaps
            .push(bitmap.to_vec().into_boxed_slice());
        self.retained_bitmaps
            .last()
            .expect("retained_bitmaps should contain the bitmap that was just pushed")
    }

    fn wrap_item(&self, ptr: *mut raw::vision_ui_list_item_t) -> Result<Item, Error> {
        NonNull::new(ptr)
            .map(Item)
            .ok_or(Error::AllocationFailed("vision_ui_list_item_t"))
    }
}

#[derive(Clone)]
pub struct UiRef {
    state: Rc<RefCell<State>>,
}

impl UiRef {
    pub fn raw_ptr(&self) -> *const raw::vision_ui_t {
        self.state.borrow().raw.as_ptr()
    }

    pub fn raw_mut_ptr(&self) -> *mut raw::vision_ui_t {
        self.state.borrow().raw.as_ptr()
    }

    pub fn action(&self) -> Action {
        unsafe { raw::vision_ui_driver_action_get(self.raw_ptr()).into() }
    }

    pub fn ticks(&self) -> Duration {
        Duration::from_millis(u64::from(unsafe {
            raw::vision_ui_driver_ticks_ms_get(self.raw_ptr())
        }))
    }

    pub fn delay(&self, duration: Duration) -> Result<(), Error> {
        let ms = duration_millis_u32(duration)?;
        unsafe { raw::vision_ui_driver_delay(self.raw_ptr(), ms) }
        Ok(())
    }

    pub fn is_closed(&self) -> bool {
        unsafe { raw::vision_ui_is_exited(self.raw_ptr()) }
    }

    pub fn freezes_background(&self) -> bool {
        unsafe { raw::vision_ui_is_background_frozen(self.raw_ptr()) }
    }

    pub fn set_mini_font(&self, font: Font) {
        unsafe { raw::vision_ui_minifont_set(self.raw_mut_ptr(), font) }
    }

    pub fn set_body_font(&self, font: Font) {
        unsafe { raw::vision_ui_font_set(self.raw_mut_ptr(), font) }
    }

    pub fn set_title_font(&self, font: Font) {
        unsafe { raw::vision_ui_font_set_title(self.raw_mut_ptr(), font) }
    }

    pub fn set_subtitle_font(&self, font: Font) {
        unsafe { raw::vision_ui_font_set_subtitle(self.raw_mut_ptr(), font) }
    }

    pub fn mini_font(&self) -> Font {
        unsafe { raw::vision_ui_minifont_get(self.raw_ptr()) }
    }

    pub fn body_font(&self) -> Font {
        unsafe { raw::vision_ui_font_get(self.raw_ptr()) }
    }

    pub fn title_font(&self) -> Font {
        unsafe { raw::vision_ui_font_get_title(self.raw_ptr()) }
    }

    pub fn subtitle_font(&self) -> Font {
        unsafe { raw::vision_ui_font_get_subtitle(self.raw_ptr()) }
    }

    pub fn notify(&self, message: impl AsRef<str>, duration: Duration) -> Result<(), Error> {
        let span = duration_millis_u16(duration)?;
        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(message.as_ref())?;
        unsafe { raw::vision_ui_notification_push(state.raw.as_ptr(), ptr, span) };
        Ok(())
    }

    pub fn alert(&self, message: impl AsRef<str>, duration: Duration) -> Result<(), Error> {
        let span = duration_millis_u16(duration)?;
        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(message.as_ref())?;
        unsafe { raw::vision_ui_alert_push(state.raw.as_ptr(), ptr, span) };
        Ok(())
    }

    pub fn set_root(&self, item: Item) -> bool {
        unsafe { raw::vision_ui_root_item_set(self.raw_mut_ptr(), item.as_ptr()) }
    }

    pub fn root(&self) -> Option<Item> {
        NonNull::new(unsafe { raw::vision_ui_root_list_get(self.raw_ptr()) }).map(Item)
    }

    pub fn push(&self, parent: Item, child: Item) -> bool {
        unsafe {
            raw::vision_ui_list_push_item(self.raw_mut_ptr(), parent.as_ptr(), child.as_ptr())
        }
    }

    pub fn set_icon_pack(&self, icon_pack: IconPack) {
        unsafe { raw::vision_ui_list_icon_set(self.raw_mut_ptr(), icon_pack) }
    }

    pub fn icon_pack(&self) -> IconPack {
        unsafe { raw::vision_ui_list_icon_get_current(self.raw_ptr()) }
    }
}

pub struct VisionUi {
    state: Rc<RefCell<State>>,
}

impl VisionUi {
    pub fn new() -> Result<Self, Error> {
        let raw = NonNull::new(unsafe { raw::vision_ui_create() })
            .ok_or(Error::AllocationFailed("vision_ui_t"))?;
        Ok(Self {
            state: Rc::new(RefCell::new(State {
                raw,
                retained_strings: Vec::new(),
                retained_bitmaps: Vec::new(),
                toggle_callbacks: Vec::new(),
                slide_callbacks: Vec::new(),
                scene_callbacks: Vec::new(),
            })),
        })
    }

    pub fn ui(&self) -> UiRef {
        UiRef {
            state: Rc::clone(&self.state),
        }
    }

    pub fn raw_ptr(&self) -> *const raw::vision_ui_t {
        self.state.borrow().raw.as_ptr()
    }

    pub fn raw_mut_ptr(&mut self) -> *mut raw::vision_ui_t {
        self.state.borrow().raw.as_ptr()
    }

    pub fn initialize_runtime(&mut self) {
        unsafe { raw::vision_ui_core_init(self.raw_mut_ptr()) }
    }

    pub fn show(&mut self) {
        unsafe { raw::vision_ui_render_init(self.raw_mut_ptr()) }
    }

    pub fn render_frame(&mut self) {
        unsafe { raw::vision_ui_step_render(self.raw_mut_ptr()) }
    }

    pub fn set_startup_logo(&mut self, bitmap: StartupLogo<'_>) -> Result<(), Error> {
        let bitmap = bitmap.bitmap();
        let span = u32::try_from(bitmap.bytes().len())
            .map_err(|_| Error::BitmapTooLarge(bitmap.bytes().len()))?;
        let mut state = self.state.borrow_mut();
        let owned = bitmap.bytes().to_vec().into_boxed_slice();
        let ptr = owned.as_ptr();
        state.retained_bitmaps.push(owned);
        unsafe { raw::vision_ui_start_logo_set(state.raw.as_ptr(), ptr, span) };
        Ok(())
    }

    pub unsafe fn bind_driver_raw(&mut self, handle: NonNull<c_void>) {
        raw::vision_ui_driver_bind(self.raw_mut_ptr(), handle.as_ptr());
    }

    pub fn bind_driver<D: driver::RawHandle>(&mut self, driver: &mut D) {
        unsafe {
            raw::vision_ui_driver_bind(self.raw_mut_ptr(), driver.as_raw_handle());
        }
    }

    pub fn action(&self) -> Action {
        self.ui().action()
    }

    pub fn ticks(&self) -> Duration {
        self.ui().ticks()
    }

    pub fn delay(&self, duration: Duration) -> Result<(), Error> {
        self.ui().delay(duration)
    }

    pub fn is_closed(&self) -> bool {
        self.ui().is_closed()
    }

    pub fn freezes_background(&self) -> bool {
        self.ui().freezes_background()
    }

    pub fn set_mini_font(&mut self, font: Font) {
        self.ui().set_mini_font(font)
    }

    pub fn set_body_font(&mut self, font: Font) {
        self.ui().set_body_font(font)
    }

    pub fn set_title_font(&mut self, font: Font) {
        self.ui().set_title_font(font)
    }

    pub fn set_subtitle_font(&mut self, font: Font) {
        self.ui().set_subtitle_font(font)
    }

    pub fn mini_font(&self) -> Font {
        self.ui().mini_font()
    }

    pub fn body_font(&self) -> Font {
        self.ui().body_font()
    }

    pub fn title_font(&self) -> Font {
        self.ui().title_font()
    }

    pub fn subtitle_font(&self) -> Font {
        self.ui().subtitle_font()
    }

    pub fn notify(&mut self, message: impl AsRef<str>, duration: Duration) -> Result<(), Error> {
        self.ui().notify(message, duration)
    }

    pub fn alert(&mut self, message: impl AsRef<str>, duration: Duration) -> Result<(), Error> {
        self.ui().alert(message, duration)
    }

    pub fn list(&mut self, title: impl AsRef<str>, capacity: usize) -> Result<Item, Error> {
        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(title.as_ref())?;
        let raw = unsafe { raw::vision_ui_list_item_new(state.raw.as_ptr(), capacity, false, ptr) };
        state.wrap_item(raw)
    }

    pub fn icon_list(&mut self, title: impl AsRef<str>, capacity: usize) -> Result<Item, Error> {
        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(title.as_ref())?;
        let raw = unsafe { raw::vision_ui_list_item_new(state.raw.as_ptr(), capacity, true, ptr) };
        state.wrap_item(raw)
    }

    pub fn title(&mut self, text: impl AsRef<str>) -> Result<Item, Error> {
        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(text.as_ref())?;
        state.wrap_item(unsafe { raw::vision_ui_list_title_item_new(state.raw.as_ptr(), ptr) })
    }

    pub fn icon(
        &mut self,
        title: impl AsRef<str>,
        description: Option<impl AsRef<str>>,
        icon: Option<MonoBitmap<'_>>,
        capacity: usize,
    ) -> Result<Item, Error> {
        let mut state = self.state.borrow_mut();
        let title_ptr = state.retain_c_string(title.as_ref())?;
        let description_ptr = match description {
            Some(text) => state.retain_c_string(text.as_ref())?,
            None => std::ptr::null(),
        };
        let icon_ptr = match icon {
            Some(bitmap) => {
                ensure_icon_bitmap(bitmap)?;
                state.retain_bitmap(bitmap.bytes()).as_ptr()
            }
            None => std::ptr::null(),
        };
        let raw = unsafe {
            raw::vision_ui_list_icon_item_new(
                state.raw.as_ptr(),
                capacity,
                icon_ptr,
                title_ptr,
                description_ptr,
            )
        };
        state.wrap_item(raw)
    }

    pub fn switch(&mut self, label: impl AsRef<str>, initial: bool) -> Result<Item, Error> {
        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(label.as_ref())?;
        let raw = unsafe {
            raw::vision_ui_list_switch_item_new(
                state.raw.as_ptr(),
                ptr,
                initial,
                None,
                std::ptr::null_mut(),
            )
        };
        state.wrap_item(raw)
    }

    pub fn switch_with<F>(
        &mut self,
        label: impl AsRef<str>,
        initial: bool,
        on_changed: F,
    ) -> Result<Item, Error>
    where
        F: FnMut(UiRef, bool) + 'static,
    {
        let callback = Box::new(ToggleCallback {
            state: Rc::downgrade(&self.state),
            handler: Box::new(on_changed),
        });
        let user_data = (&*callback as *const ToggleCallback)
            .cast_mut()
            .cast::<c_void>();

        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(label.as_ref())?;
        let raw = unsafe {
            raw::vision_ui_list_switch_item_new(
                state.raw.as_ptr(),
                ptr,
                initial,
                Some(toggle_trampoline),
                user_data,
            )
        };
        let item = state.wrap_item(raw)?;
        state.toggle_callbacks.push(callback);
        Ok(item)
    }

    pub fn slider(
        &mut self,
        label: impl AsRef<str>,
        initial: i16,
        step: u8,
        range: std::ops::RangeInclusive<i16>,
    ) -> Result<Item, Error> {
        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(label.as_ref())?;
        let min = *range.start();
        let max = *range.end();
        let raw = unsafe {
            raw::vision_ui_list_slider_item_new(
                state.raw.as_ptr(),
                ptr,
                initial,
                step,
                min,
                max,
                None,
                std::ptr::null_mut(),
            )
        };
        state.wrap_item(raw)
    }

    pub fn slider_with<F>(
        &mut self,
        label: impl AsRef<str>,
        initial: i16,
        step: u8,
        range: std::ops::RangeInclusive<i16>,
        on_changed: F,
    ) -> Result<Item, Error>
    where
        F: FnMut(UiRef, i16) + 'static,
    {
        let callback = Box::new(SlideCallback {
            state: Rc::downgrade(&self.state),
            handler: Box::new(on_changed),
        });
        let user_data = (&*callback as *const SlideCallback)
            .cast_mut()
            .cast::<c_void>();

        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(label.as_ref())?;
        let min = *range.start();
        let max = *range.end();
        let raw = unsafe {
            raw::vision_ui_list_slider_item_new(
                state.raw.as_ptr(),
                ptr,
                initial,
                step,
                min,
                max,
                Some(slide_trampoline),
                user_data,
            )
        };
        let item = state.wrap_item(raw)?;
        state.slide_callbacks.push(callback);
        Ok(item)
    }

    pub fn scene(&mut self, label: impl AsRef<str>) -> Result<Item, Error> {
        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(label.as_ref())?;
        let raw = unsafe {
            raw::vision_ui_list_user_item_new(
                state.raw.as_ptr(),
                ptr,
                None,
                None,
                None,
                std::ptr::null_mut(),
            )
        };
        state.wrap_item(raw)
    }

    pub fn scene_with<FI, FR, FE>(
        &mut self,
        label: impl AsRef<str>,
        init: Option<FI>,
        render: Option<FR>,
        exit: Option<FE>,
    ) -> Result<Item, Error>
    where
        FI: FnMut(UiRef) + 'static,
        FR: FnMut(UiRef) + 'static,
        FE: FnMut(UiRef) + 'static,
    {
        let callback = Box::new(SceneCallbackSet {
            state: Rc::downgrade(&self.state),
            init: init.map(|f| Box::new(f) as Box<SceneFn>),
            render: render.map(|f| Box::new(f) as Box<SceneFn>),
            exit: exit.map(|f| Box::new(f) as Box<SceneFn>),
        });
        let user_data = (&*callback as *const SceneCallbackSet)
            .cast_mut()
            .cast::<c_void>();

        let mut state = self.state.borrow_mut();
        let ptr = state.retain_c_string(label.as_ref())?;
        let raw = unsafe {
            raw::vision_ui_list_user_item_new(
                state.raw.as_ptr(),
                ptr,
                callback.init.as_ref().map(|_| scene_init_trampoline as _),
                callback
                    .render
                    .as_ref()
                    .map(|_| scene_render_trampoline as _),
                callback.exit.as_ref().map(|_| scene_exit_trampoline as _),
                user_data,
            )
        };
        let item = state.wrap_item(raw)?;
        state.scene_callbacks.push(callback);
        Ok(item)
    }

    pub fn set_root(&mut self, item: Item) -> bool {
        self.ui().set_root(item)
    }

    pub fn root(&self) -> Option<Item> {
        self.ui().root()
    }

    pub fn push(&mut self, parent: Item, child: Item) -> bool {
        self.ui().push(parent, child)
    }

    pub fn set_icon_pack(&mut self, icon_pack: IconPack) {
        self.ui().set_icon_pack(icon_pack)
    }

    pub fn icon_pack(&self) -> IconPack {
        self.ui().icon_pack()
    }
}

impl Drop for VisionUi {
    fn drop(&mut self) {
        unsafe { raw::vision_ui_destroy(self.raw_mut_ptr()) }
    }
}

unsafe extern "C" fn toggle_trampoline(
    _ui: *mut raw::vision_ui_t,
    value: bool,
    user_data: *mut c_void,
) {
    let callback = unsafe { &mut *user_data.cast::<ToggleCallback>() };
    let Some(state) = callback.state.upgrade() else {
        return;
    };
    (callback.handler)(UiRef { state }, value);
}

unsafe extern "C" fn slide_trampoline(
    _ui: *mut raw::vision_ui_t,
    value: i16,
    user_data: *mut c_void,
) {
    let callback = unsafe { &mut *user_data.cast::<SlideCallback>() };
    let Some(state) = callback.state.upgrade() else {
        return;
    };
    (callback.handler)(UiRef { state }, value);
}

unsafe extern "C" fn scene_init_trampoline(_ui: *mut raw::vision_ui_t, user_data: *mut c_void) {
    let callback = unsafe { &mut *user_data.cast::<SceneCallbackSet>() };
    let Some(state) = callback.state.upgrade() else {
        return;
    };
    if let Some(handler) = callback.init.as_mut() {
        handler(UiRef { state });
    }
}

unsafe extern "C" fn scene_render_trampoline(_ui: *mut raw::vision_ui_t, user_data: *mut c_void) {
    let callback = unsafe { &mut *user_data.cast::<SceneCallbackSet>() };
    let Some(state) = callback.state.upgrade() else {
        return;
    };
    if let Some(handler) = callback.render.as_mut() {
        handler(UiRef { state });
    }
}

unsafe extern "C" fn scene_exit_trampoline(_ui: *mut raw::vision_ui_t, user_data: *mut c_void) {
    let callback = unsafe { &mut *user_data.cast::<SceneCallbackSet>() };
    let Some(state) = callback.state.upgrade() else {
        return;
    };
    if let Some(handler) = callback.exit.as_mut() {
        handler(UiRef { state });
    }
}

fn duration_millis_u16(duration: Duration) -> Result<u16, Error> {
    let millis = duration.as_millis();
    u16::try_from(millis).map_err(|_| Error::DurationOverflow(duration))
}

fn duration_millis_u32(duration: Duration) -> Result<u32, Error> {
    let millis = duration.as_millis();
    u32::try_from(millis).map_err(|_| Error::DurationOverflow(duration))
}

fn bitmap_len(width: u16, height: u16) -> usize {
    usize::from(width).div_ceil(8) * usize::from(height)
}

fn ensure_icon_bitmap(bitmap: MonoBitmap<'_>) -> Result<(), Error> {
    if bitmap.width() == ICON_SIZE && bitmap.height() == ICON_SIZE {
        return Ok(());
    }

    Err(Error::BitmapDimensionsMismatch {
        expected_width: ICON_SIZE,
        expected_height: ICON_SIZE,
        actual_width: bitmap.width(),
        actual_height: bitmap.height(),
    })
}
