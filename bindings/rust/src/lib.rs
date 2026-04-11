#![no_std]

#[cfg(feature = "alloc")]
extern crate alloc;

#[cfg(feature = "alloc")]
use alloc::boxed::Box;
#[cfg(feature = "alloc")]
use alloc::vec::Vec;
use core::ffi::{c_char, c_void, CStr};
use core::ops::RangeInclusive;
use core::ptr::{self, NonNull};
use core::time::Duration;
use thiserror::Error;

pub mod config;
pub mod driver;
pub mod raw;

pub use config::{
    ALERT, ALLOW_EXIT_BY_USER, DEBUG_OVERLAY, DISPLAY, ICON_SIZE, ICON_VIEW, LIST_VIEW,
    NOTIFICATION, SCREEN_HEIGHT, SCREEN_WIDTH, SYSTEM,
};
pub use raw::vision_ui_font_t as Font;
pub use raw::vision_ui_icon_t as IconPack;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Action {
    None,
    Previous,
    Next,
    Enter,
    Exit,
}

#[derive(Debug, Error, Clone, Copy, PartialEq, Eq)]
pub enum ActionError {
    #[error("unexpected action code {0}")]
    Invalid(u32),
}

impl TryFrom<raw::vision_ui_action_t> for Action {
    type Error = ActionError;

    fn try_from(value: raw::vision_ui_action_t) -> Result<Self, Self::Error> {
        match value {
            raw::vision_ui_action_t_UiActionNone => Ok(Self::None),
            raw::vision_ui_action_t_UiActionGoPrev => Ok(Self::Previous),
            raw::vision_ui_action_t_UiActionGoNext => Ok(Self::Next),
            raw::vision_ui_action_t_UiActionEnter => Ok(Self::Enter),
            raw::vision_ui_action_t_UiActionExit => Ok(Self::Exit),
            other => Err(ActionError::Invalid(other)),
        }
    }
}

#[derive(Debug, Error, Clone, Copy, PartialEq, Eq)]
pub enum AllocOpError {
    #[error("unexpected allocation op code {0}")]
    Invalid(u32),
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AllocOp {
    Malloc,
    Calloc,
    Free,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AllocRequest {
    Malloc { size: usize },
    Calloc { size: usize, count: usize },
    Free { ptr: *mut c_void },
}

impl TryFrom<raw::vision_alloc_op_t> for AllocOp {
    type Error = AllocOpError;

    fn try_from(value: raw::vision_alloc_op_t) -> Result<Self, Self::Error> {
        match value {
            raw::vision_alloc_op_t_VisionAllocMalloc => Ok(Self::Malloc),
            raw::vision_alloc_op_t_VisionAllocCalloc => Ok(Self::Calloc),
            raw::vision_alloc_op_t_VisionAllocFree => Ok(Self::Free),
            other => Err(AllocOpError::Invalid(other)),
        }
    }
}

pub unsafe trait Allocator {
    fn allocate(request: AllocRequest) -> *mut c_void;
}

#[derive(Debug, Error, Clone, Copy, PartialEq, Eq)]
pub enum InitError {
    #[error("a root item must be attached before initializing the runtime")]
    RootItemNotSet,
    #[error("unexpected runtime initialization result code {0}")]
    Unknown(u32),
}

#[derive(Debug, Error, Clone, Copy, PartialEq, Eq)]
pub enum SetRootError {
    #[error("the root item pointer was invalid")]
    ItemInvalid,
    #[error("unexpected root-item result code {0}")]
    Unknown(u32),
}

#[derive(Debug, Error, Clone, Copy, PartialEq, Eq)]
pub enum PushItemError {
    #[error("the parent item or child item pointer was invalid")]
    ItemInvalid,
    #[error("the parent item is already full")]
    ParentFull,
    #[error("pushing this child would exceed the maximum supported nesting depth")]
    MaxLayerExceeded,
    #[error("icon-view parents only accept icon items as direct children")]
    IconViewChildMismatch,
    #[error("unexpected list-push result code {0}")]
    Unknown(u32),
}

#[derive(Debug, Error)]
pub enum Error {
    #[error("allocation failed for {0}")]
    AllocationFailed(&'static str),
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
pub struct Text(&'static CStr);

impl Text {
    pub const fn new(value: &'static CStr) -> Self {
        Self(value)
    }

    pub fn as_ptr(self) -> *const c_char {
        self.0.as_ptr()
    }

    pub fn as_c_str(self) -> &'static CStr {
        self.0
    }
}

impl From<&'static CStr> for Text {
    fn from(value: &'static CStr) -> Self {
        Self::new(value)
    }
}

#[macro_export]
macro_rules! text {
    ($lit:literal) => {{
        $crate::Text::new(
            ::core::ffi::CStr::from_bytes_with_nul(concat!($lit, "\0").as_bytes())
                .expect("text!() requires a string literal without interior NUL"),
        )
    }};
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct MonoBitmap {
    bytes: &'static [u8],
    width: u16,
    height: u16,
}

impl MonoBitmap {
    pub fn new(bytes: &'static [u8], width: u16, height: u16) -> Result<Self, Error> {
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

    pub fn bytes(self) -> &'static [u8] {
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
pub struct StartupLogo(MonoBitmap);

impl StartupLogo {
    pub fn new(bytes: &'static [u8]) -> Result<Self, Error> {
        Ok(Self(MonoBitmap::new(bytes, SCREEN_WIDTH, SCREEN_HEIGHT)?))
    }

    pub fn bitmap(self) -> MonoBitmap {
        self.0
    }
}

pub struct ToggleBinding<T: 'static> {
    context: &'static T,
    handler: fn(UiRef, bool, &'static T),
}

impl<T: 'static> ToggleBinding<T> {
    pub const fn new(context: &'static T, handler: fn(UiRef, bool, &'static T)) -> Self {
        Self { context, handler }
    }
}

pub struct SlideBinding<T: 'static> {
    context: &'static T,
    handler: fn(UiRef, i16, &'static T),
}

impl<T: 'static> SlideBinding<T> {
    pub const fn new(context: &'static T, handler: fn(UiRef, i16, &'static T)) -> Self {
        Self { context, handler }
    }
}

pub struct SceneBindings<T: 'static> {
    context: &'static T,
    init: Option<fn(UiRef, &'static T)>,
    render: Option<fn(UiRef, &'static T)>,
    exit: Option<fn(UiRef, &'static T)>,
}

impl<T: 'static> SceneBindings<T> {
    pub const fn new(
        context: &'static T,
        init: Option<fn(UiRef, &'static T)>,
        render: Option<fn(UiRef, &'static T)>,
        exit: Option<fn(UiRef, &'static T)>,
    ) -> Self {
        Self {
            context,
            init,
            render,
            exit,
        }
    }
}

#[cfg(feature = "alloc")]
type ToggleClosureFn = dyn FnMut(UiRef, bool);
#[cfg(feature = "alloc")]
type SlideClosureFn = dyn FnMut(UiRef, i16);
#[cfg(feature = "alloc")]
type SceneClosureFn = dyn FnMut(UiRef);

#[cfg(feature = "alloc")]
struct ToggleClosure {
    handler: Box<ToggleClosureFn>,
}

#[cfg(feature = "alloc")]
struct SlideClosure {
    handler: Box<SlideClosureFn>,
}

#[cfg(feature = "alloc")]
struct SceneClosureBindings {
    init: Option<Box<SceneClosureFn>>,
    render: Option<Box<SceneClosureFn>>,
    exit: Option<Box<SceneClosureFn>>,
}

#[derive(Clone)]
pub struct UiRef {
    raw: NonNull<raw::vision_ui_t>,
}

impl UiRef {
    pub fn raw_ptr(&self) -> *const raw::vision_ui_t {
        self.raw.as_ptr()
    }

    pub fn raw_mut_ptr(&self) -> *mut raw::vision_ui_t {
        self.raw.as_ptr()
    }

    pub fn action(&self) -> Result<Action, ActionError> {
        unsafe { raw::vision_ui_driver_action_get(self.raw_ptr()).try_into() }
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

    pub fn notify(&self, message: Text, duration: Duration) -> Result<(), Error> {
        let span = duration_millis_u16(duration)?;
        unsafe { raw::vision_ui_notification_push(self.raw.as_ptr(), message.as_ptr(), span) };
        Ok(())
    }

    pub fn alert(&self, message: Text, duration: Duration) -> Result<(), Error> {
        let span = duration_millis_u16(duration)?;
        unsafe { raw::vision_ui_alert_push(self.raw.as_ptr(), message.as_ptr(), span) };
        Ok(())
    }

    pub fn set_root(&self, item: Item) -> Result<(), SetRootError> {
        match unsafe { raw::vision_ui_root_item_set(self.raw_mut_ptr(), item.as_ptr()) } {
            raw::vision_ui_root_item_set_result_t_VisionUiRootItemSetOk => Ok(()),
            raw::vision_ui_root_item_set_result_t_VisionUiRootItemSetItemInvalid => {
                Err(SetRootError::ItemInvalid)
            }
            other => Err(SetRootError::Unknown(other)),
        }
    }

    pub fn root(&self) -> Option<Item> {
        NonNull::new(unsafe { raw::vision_ui_root_list_get(self.raw_ptr()) }).map(Item)
    }

    pub fn push(&self, parent: Item, child: Item) -> Result<(), PushItemError> {
        match unsafe {
            raw::vision_ui_list_push_item(self.raw_mut_ptr(), parent.as_ptr(), child.as_ptr())
        } {
            raw::vision_ui_list_push_item_result_t_VisionUiListPushItemOk => Ok(()),
            raw::vision_ui_list_push_item_result_t_VisionUiListPushItemItemInvalid => {
                Err(PushItemError::ItemInvalid)
            }
            raw::vision_ui_list_push_item_result_t_VisionUiListPushItemParentFull => {
                Err(PushItemError::ParentFull)
            }
            raw::vision_ui_list_push_item_result_t_VisionUiListPushItemMaxLayerExceeded => {
                Err(PushItemError::MaxLayerExceeded)
            }
            raw::vision_ui_list_push_item_result_t_VisionUiListPushItemIconViewChildMismatch => {
                Err(PushItemError::IconViewChildMismatch)
            }
            other => Err(PushItemError::Unknown(other)),
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
    raw: NonNull<raw::vision_ui_t>,
    #[cfg(feature = "alloc")]
    toggle_closures: Vec<Box<ToggleClosure>>,
    #[cfg(feature = "alloc")]
    slide_closures: Vec<Box<SlideClosure>>,
    #[cfg(feature = "alloc")]
    scene_closures: Vec<Box<SceneClosureBindings>>,
}

impl VisionUi {
    pub fn new() -> Result<Self, Error> {
        let raw = NonNull::new(unsafe { raw::vision_ui_create() })
            .ok_or(Error::AllocationFailed("vision_ui_t"))?;
        Ok(Self {
            raw,
            #[cfg(feature = "alloc")]
            toggle_closures: Vec::new(),
            #[cfg(feature = "alloc")]
            slide_closures: Vec::new(),
            #[cfg(feature = "alloc")]
            scene_closures: Vec::new(),
        })
    }

    pub fn ui(&self) -> UiRef {
        UiRef { raw: self.raw }
    }

    pub fn raw_ptr(&self) -> *const raw::vision_ui_t {
        self.raw.as_ptr()
    }

    pub fn raw_mut_ptr(&mut self) -> *mut raw::vision_ui_t {
        self.raw.as_ptr()
    }

    pub fn initialize_runtime(&mut self) -> Result<(), InitError> {
        match unsafe { raw::vision_ui_core_init(self.raw_mut_ptr()) } {
            raw::vision_ui_core_init_result_t_VisionUiCoreInitOk => Ok(()),
            raw::vision_ui_core_init_result_t_VisionUiCoreInitRootItemNotSet => {
                Err(InitError::RootItemNotSet)
            }
            other => Err(InitError::Unknown(other)),
        }
    }

    pub fn initialize_rendering(&mut self) {
        unsafe { raw::vision_ui_render_init(self.raw_mut_ptr()) }
    }

    pub fn render_frame(&mut self) {
        unsafe { raw::vision_ui_step_render(self.raw_mut_ptr()) }
    }

    pub fn set_startup_logo(&mut self, bitmap: StartupLogo) -> Result<(), Error> {
        let bitmap = bitmap.bitmap();
        let span = u32::try_from(bitmap.bytes().len())
            .map_err(|_| Error::BitmapTooLarge(bitmap.bytes().len()))?;
        unsafe { raw::vision_ui_start_logo_set(self.raw.as_ptr(), bitmap.bytes().as_ptr(), span) };
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

    pub fn action(&self) -> Result<Action, ActionError> {
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

    pub fn set_allocator<A: Allocator>(&mut self) {
        unsafe { raw::vision_ui_allocator_set(self.raw.as_ptr(), Some(allocator_trampoline::<A>)) }
    }

    pub unsafe fn set_allocator_raw(&mut self, allocator: raw::vision_ui_allocator_t) {
        unsafe { raw::vision_ui_allocator_set(self.raw.as_ptr(), allocator) }
    }

    pub fn clear_allocator(&mut self) {
        unsafe { raw::vision_ui_allocator_set(self.raw.as_ptr(), None) }
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

    pub fn notify(&mut self, message: Text, duration: Duration) -> Result<(), Error> {
        self.ui().notify(message, duration)
    }

    pub fn alert(&mut self, message: Text, duration: Duration) -> Result<(), Error> {
        self.ui().alert(message, duration)
    }

    pub fn list(&mut self, title: Text, capacity: usize) -> Result<Item, Error> {
        wrap_item(unsafe {
            raw::vision_ui_list_item_new(self.raw.as_ptr(), capacity, false, title.as_ptr())
        })
    }

    pub fn icon_list(&mut self, title: Text, capacity: usize) -> Result<Item, Error> {
        wrap_item(unsafe {
            raw::vision_ui_list_item_new(self.raw.as_ptr(), capacity, true, title.as_ptr())
        })
    }

    pub fn title(&mut self, text: Text) -> Result<Item, Error> {
        wrap_item(unsafe { raw::vision_ui_list_title_item_new(self.raw.as_ptr(), text.as_ptr()) })
    }

    pub fn icon(
        &mut self,
        title: Text,
        description: Option<Text>,
        icon: Option<MonoBitmap>,
        capacity: usize,
    ) -> Result<Item, Error> {
        let title_ptr = title.as_ptr();
        let description_ptr = match description {
            Some(text) => text.as_ptr(),
            None => ptr::null(),
        };
        let icon_ptr = match icon {
            Some(bitmap) => {
                ensure_icon_bitmap(bitmap)?;
                bitmap.bytes().as_ptr()
            }
            None => ptr::null(),
        };
        let raw = unsafe {
            raw::vision_ui_list_icon_item_new(
                self.raw.as_ptr(),
                capacity,
                icon_ptr,
                title_ptr,
                description_ptr,
            )
        };
        wrap_item(raw)
    }

    pub fn switch(&mut self, label: Text, initial: bool) -> Result<Item, Error> {
        let raw = unsafe {
            raw::vision_ui_list_switch_item_new(
                self.raw.as_ptr(),
                label.as_ptr(),
                initial,
                None,
                ptr::null_mut(),
            )
        };
        wrap_item(raw)
    }

    pub fn switch_with<T: 'static>(
        &mut self,
        label: Text,
        initial: bool,
        binding: &'static ToggleBinding<T>,
    ) -> Result<Item, Error> {
        let raw = unsafe {
            raw::vision_ui_list_switch_item_new(
                self.raw.as_ptr(),
                label.as_ptr(),
                initial,
                Some(toggle_trampoline::<T>),
                (binding as *const ToggleBinding<T>)
                    .cast_mut()
                    .cast::<c_void>(),
            )
        };
        wrap_item(raw)
    }

    #[cfg(feature = "alloc")]
    pub fn switch_with_closure<F>(
        &mut self,
        label: Text,
        initial: bool,
        on_changed: F,
    ) -> Result<Item, Error>
    where
        F: FnMut(UiRef, bool) + 'static,
    {
        let callback = Box::new(ToggleClosure {
            handler: Box::new(on_changed),
        });
        let user_data = (&*callback as *const ToggleClosure)
            .cast_mut()
            .cast::<c_void>();
        let raw = unsafe {
            raw::vision_ui_list_switch_item_new(
                self.raw.as_ptr(),
                label.as_ptr(),
                initial,
                Some(toggle_closure_trampoline),
                user_data,
            )
        };
        let item = wrap_item(raw)?;
        self.toggle_closures.push(callback);
        Ok(item)
    }

    pub fn slider(
        &mut self,
        label: Text,
        initial: i16,
        step: u8,
        range: RangeInclusive<i16>,
    ) -> Result<Item, Error> {
        let min = *range.start();
        let max = *range.end();
        let raw = unsafe {
            raw::vision_ui_list_slider_item_new(
                self.raw.as_ptr(),
                label.as_ptr(),
                initial,
                step,
                min,
                max,
                None,
                ptr::null_mut(),
            )
        };
        wrap_item(raw)
    }

    pub fn slider_with<T: 'static>(
        &mut self,
        label: Text,
        initial: i16,
        step: u8,
        range: RangeInclusive<i16>,
        binding: &'static SlideBinding<T>,
    ) -> Result<Item, Error> {
        let min = *range.start();
        let max = *range.end();
        let raw = unsafe {
            raw::vision_ui_list_slider_item_new(
                self.raw.as_ptr(),
                label.as_ptr(),
                initial,
                step,
                min,
                max,
                Some(slide_trampoline::<T>),
                (binding as *const SlideBinding<T>)
                    .cast_mut()
                    .cast::<c_void>(),
            )
        };
        wrap_item(raw)
    }

    #[cfg(feature = "alloc")]
    pub fn slider_with_closure<F>(
        &mut self,
        label: Text,
        initial: i16,
        step: u8,
        range: RangeInclusive<i16>,
        on_changed: F,
    ) -> Result<Item, Error>
    where
        F: FnMut(UiRef, i16) + 'static,
    {
        let callback = Box::new(SlideClosure {
            handler: Box::new(on_changed),
        });
        let user_data = (&*callback as *const SlideClosure)
            .cast_mut()
            .cast::<c_void>();
        let min = *range.start();
        let max = *range.end();
        let raw = unsafe {
            raw::vision_ui_list_slider_item_new(
                self.raw.as_ptr(),
                label.as_ptr(),
                initial,
                step,
                min,
                max,
                Some(slide_closure_trampoline),
                user_data,
            )
        };
        let item = wrap_item(raw)?;
        self.slide_closures.push(callback);
        Ok(item)
    }

    pub fn scene(&mut self, label: Text) -> Result<Item, Error> {
        let raw = unsafe {
            raw::vision_ui_list_user_item_new(
                self.raw.as_ptr(),
                label.as_ptr(),
                None,
                None,
                None,
                ptr::null_mut(),
            )
        };
        wrap_item(raw)
    }

    pub fn scene_with<T: 'static>(
        &mut self,
        label: Text,
        bindings: &'static SceneBindings<T>,
    ) -> Result<Item, Error> {
        let raw = unsafe {
            raw::vision_ui_list_user_item_new(
                self.raw.as_ptr(),
                label.as_ptr(),
                bindings.init.map(|_| scene_init_trampoline::<T> as _),
                bindings.render.map(|_| scene_render_trampoline::<T> as _),
                bindings.exit.map(|_| scene_exit_trampoline::<T> as _),
                (bindings as *const SceneBindings<T>)
                    .cast_mut()
                    .cast::<c_void>(),
            )
        };
        wrap_item(raw)
    }

    #[cfg(feature = "alloc")]
    pub fn scene_with_closure<FI, FR, FE>(
        &mut self,
        label: Text,
        init: Option<FI>,
        render: Option<FR>,
        exit: Option<FE>,
    ) -> Result<Item, Error>
    where
        FI: FnMut(UiRef) + 'static,
        FR: FnMut(UiRef) + 'static,
        FE: FnMut(UiRef) + 'static,
    {
        let bindings = Box::new(SceneClosureBindings {
            init: init.map(|f| Box::new(f) as Box<SceneClosureFn>),
            render: render.map(|f| Box::new(f) as Box<SceneClosureFn>),
            exit: exit.map(|f| Box::new(f) as Box<SceneClosureFn>),
        });
        let user_data = (&*bindings as *const SceneClosureBindings)
            .cast_mut()
            .cast::<c_void>();
        let raw = unsafe {
            raw::vision_ui_list_user_item_new(
                self.raw.as_ptr(),
                label.as_ptr(),
                bindings
                    .init
                    .as_ref()
                    .map(|_| scene_init_closure_trampoline as _),
                bindings
                    .render
                    .as_ref()
                    .map(|_| scene_render_closure_trampoline as _),
                bindings
                    .exit
                    .as_ref()
                    .map(|_| scene_exit_closure_trampoline as _),
                user_data,
            )
        };
        let item = wrap_item(raw)?;
        self.scene_closures.push(bindings);
        Ok(item)
    }

    pub fn set_root(&mut self, item: Item) -> Result<(), SetRootError> {
        self.ui().set_root(item)
    }

    pub fn root(&self) -> Option<Item> {
        self.ui().root()
    }

    pub fn push(&mut self, parent: Item, child: Item) -> Result<(), PushItemError> {
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

unsafe extern "C" fn toggle_trampoline<T: 'static>(
    ui: *mut raw::vision_ui_t,
    value: bool,
    user_data: *mut c_void,
) {
    let binding = unsafe { &*user_data.cast::<ToggleBinding<T>>() };
    (binding.handler)(
        UiRef {
            raw: NonNull::new_unchecked(ui),
        },
        value,
        binding.context,
    );
}

unsafe extern "C" fn slide_trampoline<T: 'static>(
    ui: *mut raw::vision_ui_t,
    value: i16,
    user_data: *mut c_void,
) {
    let binding = unsafe { &*user_data.cast::<SlideBinding<T>>() };
    (binding.handler)(
        UiRef {
            raw: NonNull::new_unchecked(ui),
        },
        value,
        binding.context,
    );
}

unsafe extern "C" fn scene_init_trampoline<T: 'static>(
    ui: *mut raw::vision_ui_t,
    user_data: *mut c_void,
) {
    let bindings = unsafe { &*user_data.cast::<SceneBindings<T>>() };
    if let Some(handler) = bindings.init {
        handler(
            UiRef {
                raw: NonNull::new_unchecked(ui),
            },
            bindings.context,
        );
    }
}

unsafe extern "C" fn scene_render_trampoline<T: 'static>(
    ui: *mut raw::vision_ui_t,
    user_data: *mut c_void,
) {
    let bindings = unsafe { &*user_data.cast::<SceneBindings<T>>() };
    if let Some(handler) = bindings.render {
        handler(
            UiRef {
                raw: NonNull::new_unchecked(ui),
            },
            bindings.context,
        );
    }
}

unsafe extern "C" fn scene_exit_trampoline<T: 'static>(
    _ui: *mut raw::vision_ui_t,
    user_data: *mut c_void,
) {
    let bindings = unsafe { &*user_data.cast::<SceneBindings<T>>() };
    if let Some(handler) = bindings.exit {
        handler(
            UiRef {
                raw: NonNull::new_unchecked(_ui),
            },
            bindings.context,
        );
    }
}

#[cfg(feature = "alloc")]
unsafe extern "C" fn toggle_closure_trampoline(
    ui: *mut raw::vision_ui_t,
    value: bool,
    user_data: *mut c_void,
) {
    let callback = unsafe { &mut *user_data.cast::<ToggleClosure>() };
    (callback.handler)(
        UiRef {
            raw: NonNull::new_unchecked(ui),
        },
        value,
    );
}

#[cfg(feature = "alloc")]
unsafe extern "C" fn slide_closure_trampoline(
    ui: *mut raw::vision_ui_t,
    value: i16,
    user_data: *mut c_void,
) {
    let callback = unsafe { &mut *user_data.cast::<SlideClosure>() };
    (callback.handler)(
        UiRef {
            raw: NonNull::new_unchecked(ui),
        },
        value,
    );
}

#[cfg(feature = "alloc")]
unsafe extern "C" fn scene_init_closure_trampoline(
    ui: *mut raw::vision_ui_t,
    user_data: *mut c_void,
) {
    let bindings = unsafe { &mut *user_data.cast::<SceneClosureBindings>() };
    if let Some(handler) = bindings.init.as_mut() {
        handler(UiRef {
            raw: NonNull::new_unchecked(ui),
        });
    }
}

#[cfg(feature = "alloc")]
unsafe extern "C" fn scene_render_closure_trampoline(
    ui: *mut raw::vision_ui_t,
    user_data: *mut c_void,
) {
    let bindings = unsafe { &mut *user_data.cast::<SceneClosureBindings>() };
    if let Some(handler) = bindings.render.as_mut() {
        handler(UiRef {
            raw: NonNull::new_unchecked(ui),
        });
    }
}

#[cfg(feature = "alloc")]
unsafe extern "C" fn scene_exit_closure_trampoline(
    ui: *mut raw::vision_ui_t,
    user_data: *mut c_void,
) {
    let bindings = unsafe { &mut *user_data.cast::<SceneClosureBindings>() };
    if let Some(handler) = bindings.exit.as_mut() {
        handler(UiRef {
            raw: NonNull::new_unchecked(ui),
        });
    }
}

fn wrap_item(ptr: *mut raw::vision_ui_list_item_t) -> Result<Item, Error> {
    NonNull::new(ptr)
        .map(Item)
        .ok_or(Error::AllocationFailed("vision_ui_list_item_t"))
}

unsafe extern "C" fn allocator_trampoline<A: Allocator>(
    op: raw::vision_alloc_op_t,
    size: usize,
    count: usize,
    ptr: *mut c_void,
) -> *mut c_void {
    let request = match op {
        raw::vision_alloc_op_t_VisionAllocMalloc => AllocRequest::Malloc { size },
        raw::vision_alloc_op_t_VisionAllocCalloc => AllocRequest::Calloc { size, count },
        raw::vision_alloc_op_t_VisionAllocFree => AllocRequest::Free { ptr },
        _ => return ptr::null_mut(),
    };
    A::allocate(request)
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

fn ensure_icon_bitmap(bitmap: MonoBitmap) -> Result<(), Error> {
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
