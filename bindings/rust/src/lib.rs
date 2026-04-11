#![no_std]

#[cfg(feature = "alloc")]
extern crate alloc;

#[cfg(feature = "alloc")]
use alloc::boxed::Box;
#[cfg(feature = "alloc")]
use alloc::vec::Vec;
use core::ffi::{c_char, c_void, CStr};
use core::mem::MaybeUninit;
use core::ops::RangeInclusive;
use core::ptr::{self, NonNull};
use core::slice;
use core::time::Duration;
use thiserror::Error;

pub mod config;
pub mod driver;
pub mod raw;

pub use config::{
    ALERT, ALLOW_EXIT_BY_USER, DEBUG_OVERLAY, DISPLAY, ICON_SIZE, ICON_VIEW, LIST_VIEW,
    NOTIFICATION, SCREEN_HEIGHT, SCREEN_WIDTH, SYSTEM,
};
/// Font configuration used by Vision UI and backend drivers.
pub use raw::vision_ui_font_t as Font;
/// Shared list icon pack used by the renderer.
pub use raw::vision_ui_icon_t as IconPack;

/// Returns the built-in list icon pack used by the native renderer defaults.
pub fn default_icon_pack() -> IconPack {
    unsafe { raw::DEFAULT_LIST_ICON }
}

/// High-level input action returned by the bound driver.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Action {
    None,
    Previous,
    Next,
    Enter,
    Exit,
}

/// Error returned when a backend reports an invalid action code.
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

/// Allocation operation requested by the native C core.
#[derive(Debug, Error, Clone, Copy, PartialEq, Eq)]
pub enum AllocOpError {
    #[error("unexpected allocation op code {0}")]
    Invalid(u32),
}

/// Allocation operation kind used by the native allocator callback.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AllocOp {
    Malloc,
    Calloc,
    Free,
}

/// Allocation request forwarded from the C core into a Rust allocator.
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

/// Stateless allocator hook used for subsequent library-managed allocations.
///
/// This trait is `unsafe` to implement because incorrect behavior can violate the
/// native library's allocation contract and lead to memory corruption.
pub unsafe trait Allocator {
    fn allocate(request: AllocRequest) -> *mut c_void;
}

/// Error returned by [`VisionUi::initialize_runtime`].
#[derive(Debug, Error, Clone, Copy, PartialEq, Eq)]
pub enum InitError {
    #[error("a root item must be attached before initializing the runtime")]
    RootItemNotSet,
    #[error("unexpected runtime initialization result code {0}")]
    Unknown(u32),
}

/// Error returned by [`UiRef::set_root`] and [`VisionUi::set_root`].
#[derive(Debug, Error, Clone, Copy, PartialEq, Eq)]
pub enum SetRootError {
    #[error("the root item pointer was invalid")]
    ItemInvalid,
    #[error("unexpected root-item result code {0}")]
    Unknown(u32),
}

/// Error returned by [`UiRef::push`] and [`VisionUi::push`].
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

/// General-purpose error used by the safe Rust wrapper.
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

/// Opaque handle to a list item stored inside the native UI tree.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Item(NonNull<raw::vision_ui_list_item_t>);

impl Item {
    /// Returns the raw native pointer for advanced interop.
    ///
    /// Returns:
    /// - the borrowed `vision_ui_list_item_t*` managed by the native UI tree.
    ///
    /// Behavior:
    /// - The pointer remains owned by the native Vision UI runtime.
    /// - This is mainly intended for FFI interop or bridging to [`raw`].
    pub fn as_ptr(self) -> *mut raw::vision_ui_list_item_t {
        self.0.as_ptr()
    }

    /// Returns the raw native pointer as [`NonNull`].
    ///
    /// Returns:
    /// - the same native item pointer wrapped in [`NonNull`].
    pub fn as_non_null(self) -> NonNull<raw::vision_ui_list_item_t> {
        self.0
    }
}

/// Static NUL-terminated text borrowed by the native C API.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Text(&'static CStr);

impl Text {
    /// Creates a text value from a static C string.
    ///
    /// Parameters:
    /// - `value`: static NUL-terminated text borrowed by the C API.
    ///
    /// Returns:
    /// - a lightweight `Text` wrapper borrowing `value`.
    pub const fn new(value: &'static CStr) -> Self {
        Self(value)
    }

    /// Returns the raw native pointer.
    ///
    /// Returns:
    /// - the underlying `const char*` borrowed by the C API.
    pub fn as_ptr(self) -> *const c_char {
        self.0.as_ptr()
    }

    /// Returns the underlying [`CStr`].
    ///
    /// Returns:
    /// - the original static [`CStr`] backing this text value.
    pub fn as_c_str(self) -> &'static CStr {
        self.0
    }
}

impl From<&'static CStr> for Text {
    fn from(value: &'static CStr) -> Self {
        Self::new(value)
    }
}

/// Creates a static [`Text`] value from a string literal without heap allocation.
///
/// Parameters:
/// - a Rust string literal without interior NUL bytes.
///
/// Returns:
/// - a [`Text`] value backed by static NUL-terminated storage.
///
/// Behavior:
/// - The macro appends the trailing NUL at compile time.
/// - It panics if the literal contains an interior NUL byte.
#[macro_export]
macro_rules! text {
    ($lit:literal) => {{
        $crate::Text::new(
            ::core::ffi::CStr::from_bytes_with_nul(concat!($lit, "\0").as_bytes())
                .expect("text!() requires a string literal without interior NUL"),
        )
    }};
}

/// Borrowed monochrome bitmap in the XBM-compatible format used by Vision UI.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct MonoBitmap {
    bytes: &'static [u8],
    width: u16,
    height: u16,
}

impl MonoBitmap {
    /// Validates the bitmap payload against the supplied dimensions.
    ///
    /// Parameters:
    /// - `bytes`: static XBM-compatible 1-bit bitmap payload.
    /// - `width`: bitmap width in pixels.
    /// - `height`: bitmap height in pixels.
    ///
    /// Returns:
    /// - `Ok(MonoBitmap)` when `bytes.len()` matches the expected packed payload size.
    /// - `Err(Error::InvalidBitmapLength { .. })` when the payload size is inconsistent with the dimensions.
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

    /// Returns the raw bitmap bytes.
    ///
    /// Returns:
    /// - the borrowed static bitmap payload.
    pub fn bytes(self) -> &'static [u8] {
        self.bytes
    }

    /// Returns the bitmap width.
    ///
    /// Returns:
    /// - the validated width in pixels.
    pub fn width(self) -> u16 {
        self.width
    }

    /// Returns the bitmap height.
    ///
    /// Returns:
    /// - the validated height in pixels.
    pub fn height(self) -> u16 {
        self.height
    }
}

/// Startup logo bitmap validated against the configured screen size.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct StartupLogo(MonoBitmap);

impl StartupLogo {
    /// Creates a startup logo for the configured display dimensions.
    ///
    /// Parameters:
    /// - `bytes`: static XBM-compatible bitmap payload for the full screen.
    ///
    /// Returns:
    /// - `Ok(StartupLogo)` when the payload matches the configured `SCREEN_WIDTH x SCREEN_HEIGHT`.
    /// - `Err(Error::InvalidBitmapLength { .. })` when the payload size is inconsistent with the configured screen size.
    pub fn new(bytes: &'static [u8]) -> Result<Self, Error> {
        Ok(Self(MonoBitmap::new(bytes, SCREEN_WIDTH, SCREEN_HEIGHT)?))
    }

    /// Returns the validated startup bitmap.
    ///
    /// Returns:
    /// - the underlying full-screen [`MonoBitmap`].
    pub fn bitmap(self) -> MonoBitmap {
        self.0
    }
}

/// Static callback binding for switch items.
pub struct ToggleBinding<T: 'static> {
    context: &'static T,
    handler: fn(UiRef, bool, &'static T),
}

impl<T: 'static> ToggleBinding<T> {
    /// Creates a switch callback binding with static context.
    ///
    /// Parameters:
    /// - `context`: static user context passed back into the handler.
    /// - `handler`: function invoked when the switch value changes.
    ///
    /// Returns:
    /// - a zero-allocation callback binding suitable for [`VisionUi::switch_with`].
    pub const fn new(context: &'static T, handler: fn(UiRef, bool, &'static T)) -> Self {
        Self { context, handler }
    }
}

/// Static callback binding for slider items.
pub struct SlideBinding<T: 'static> {
    context: &'static T,
    handler: fn(UiRef, i16, &'static T),
}

impl<T: 'static> SlideBinding<T> {
    /// Creates a slider callback binding with static context.
    ///
    /// Parameters:
    /// - `context`: static user context passed back into the handler.
    /// - `handler`: function invoked when the slider value changes.
    ///
    /// Returns:
    /// - a zero-allocation callback binding suitable for [`VisionUi::slider_with`].
    pub const fn new(context: &'static T, handler: fn(UiRef, i16, &'static T)) -> Self {
        Self { context, handler }
    }
}

/// Static callback set for custom scene items.
pub struct SceneBindings<T: 'static> {
    context: &'static T,
    init: Option<fn(UiRef, &'static T)>,
    render: Option<fn(UiRef, &'static T)>,
    exit: Option<fn(UiRef, &'static T)>,
}

impl<T: 'static> SceneBindings<T> {
    /// Creates a scene callback set with optional init, render, and exit hooks.
    ///
    /// Parameters:
    /// - `context`: static user context passed into every hook.
    /// - `init`: optional hook called when the scene is first entered.
    /// - `render`: optional hook called while the scene remains active.
    /// - `exit`: optional hook called when leaving the scene.
    ///
    /// Returns:
    /// - a zero-allocation callback set suitable for [`VisionUi::scene_with`].
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

/// Borrowed handle back into the active UI instance.
///
/// This is passed into item callbacks so they can interact with the running UI
/// without exposing low-level callback ABI details.
#[derive(Clone, Copy)]
pub struct UiRef {
    raw: NonNull<raw::vision_ui_t>,
}

impl UiRef {
    /// Returns the raw native pointer for advanced interop.
    ///
    /// Returns:
    /// - a shared pointer to the active native `vision_ui_t`.
    pub fn raw_ptr(&self) -> *const raw::vision_ui_t {
        self.raw.as_ptr()
    }

    /// Returns the raw mutable native pointer for advanced interop.
    ///
    /// Returns:
    /// - a mutable pointer to the active native `vision_ui_t`.
    pub fn raw_mut_ptr(&self) -> *mut raw::vision_ui_t {
        self.raw.as_ptr()
    }

    /// Returns whether the UI has completed its exit flow.
    ///
    /// Returns:
    /// - `true` when the native UI reports that it has exited.
    pub fn is_closed(&self) -> bool {
        unsafe { raw::vision_ui_is_exited(self.raw_ptr()) }
    }

    /// Returns whether background interaction should be paused behind the UI.
    ///
    /// Returns:
    /// - `true` when Vision UI wants background activity to remain frozen.
    pub fn freezes_background(&self) -> bool {
        unsafe { raw::vision_ui_is_background_frozen(self.raw_ptr()) }
    }

    /// Sets the small utility font.
    ///
    /// Parameters:
    /// - `font`: font descriptor forwarded to the native UI state.
    pub fn set_mini_font(&self, font: Font) {
        unsafe { raw::vision_ui_minifont_set(self.raw_mut_ptr(), font) }
    }

    /// Sets the main body font.
    ///
    /// Parameters:
    /// - `font`: font descriptor used for standard content rows.
    pub fn set_body_font(&self, font: Font) {
        unsafe { raw::vision_ui_font_set(self.raw_mut_ptr(), font) }
    }

    /// Sets the title font.
    ///
    /// Parameters:
    /// - `font`: font descriptor used for primary titles.
    pub fn set_title_font(&self, font: Font) {
        unsafe { raw::vision_ui_font_set_title(self.raw_mut_ptr(), font) }
    }

    /// Sets the subtitle font.
    ///
    /// Parameters:
    /// - `font`: font descriptor used for subtitle text.
    pub fn set_subtitle_font(&self, font: Font) {
        unsafe { raw::vision_ui_font_set_subtitle(self.raw_mut_ptr(), font) }
    }

    /// Returns the current small utility font.
    ///
    /// Returns:
    /// - the font currently used for compact utility text.
    pub fn mini_font(&self) -> Font {
        unsafe { raw::vision_ui_minifont_get(self.raw_ptr()) }
    }

    /// Returns the current main body font.
    ///
    /// Returns:
    /// - the font currently used for normal content rows.
    pub fn body_font(&self) -> Font {
        unsafe { raw::vision_ui_font_get(self.raw_ptr()) }
    }

    /// Returns the current title font.
    ///
    /// Returns:
    /// - the font currently used for primary titles.
    pub fn title_font(&self) -> Font {
        unsafe { raw::vision_ui_font_get_title(self.raw_ptr()) }
    }

    /// Returns the current subtitle font.
    ///
    /// Returns:
    /// - the font currently used for subtitle text.
    pub fn subtitle_font(&self) -> Font {
        unsafe { raw::vision_ui_font_get_subtitle(self.raw_ptr()) }
    }

    /// Shows a temporary notification overlay for the requested duration.
    ///
    /// Parameters:
    /// - `message`: static text borrowed by the native notification system.
    /// - `duration`: how long the notification should remain visible.
    ///
    /// Returns:
    /// - `Ok(())` when the duration fits in Vision UI's `u16` millisecond range.
    /// - `Err(Error::DurationOverflow(_))` when the duration is too large to represent.
    ///
    /// Behavior:
    /// - The notification is queued immediately on the native UI instance.
    /// - Existing notification state is handled by the C runtime.
    pub fn notify(&self, message: Text, duration: Duration) -> Result<(), Error> {
        let span = duration_millis_u16(duration)?;
        unsafe { raw::vision_ui_notification_push(self.raw.as_ptr(), message.as_ptr(), span) };
        Ok(())
    }

    /// Shows a centered alert overlay for the requested duration.
    ///
    /// Parameters:
    /// - `message`: static text borrowed by the native alert system.
    /// - `duration`: how long the alert should remain visible.
    ///
    /// Returns:
    /// - `Ok(())` when the duration fits in Vision UI's `u16` millisecond range.
    /// - `Err(Error::DurationOverflow(_))` when the duration is too large to represent.
    ///
    /// Behavior:
    /// - The alert is activated immediately on the native UI instance.
    /// - Alert presentation and dismissal timing are handled by the C runtime.
    pub fn alert(&self, message: Text, duration: Duration) -> Result<(), Error> {
        let span = duration_millis_u16(duration)?;
        unsafe { raw::vision_ui_alert_push(self.raw.as_ptr(), message.as_ptr(), span) };
        Ok(())
    }

    /// Attaches the root list shown by the UI.
    ///
    /// Parameters:
    /// - `item`: list container to use as the top-level root.
    ///
    /// Returns:
    /// - `Ok(())` when the root was accepted.
    /// - `Err(SetRootError::ItemInvalid)` when the supplied handle is invalid.
    ///
    /// Behavior:
    /// - The root pointer is stored by the native UI instance.
    /// - The tree should usually be fully constructed before [`UiRef::set_root`] or
    ///   [`VisionUi::set_root`] is followed by [`VisionUi::initialize_runtime`].
    pub fn set_root(&self, item: Item) -> Result<(), SetRootError> {
        match unsafe { raw::vision_ui_root_item_set(self.raw_mut_ptr(), item.as_ptr()) } {
            raw::vision_ui_root_item_set_result_t_VisionUiRootItemSetOk => Ok(()),
            raw::vision_ui_root_item_set_result_t_VisionUiRootItemSetItemInvalid => {
                Err(SetRootError::ItemInvalid)
            }
            other => Err(SetRootError::Unknown(other)),
        }
    }

    /// Returns the current root item, if one has been attached.
    ///
    /// Returns:
    /// - `Some(Item)` when a root item is attached.
    /// - `None` when no root item has been set yet.
    pub fn root(&self) -> Option<Item> {
        NonNull::new(unsafe { raw::vision_ui_root_list_get(self.raw_ptr()) }).map(Item)
    }

    /// Appends a child item to a parent list or icon-view container.
    ///
    /// Parameters:
    /// - `parent`: destination list or icon-view item.
    /// - `child`: child item appended directly under `parent`.
    ///
    /// Returns:
    /// - `Ok(())` when the child was appended successfully.
    /// - `Err(PushItemError::ItemInvalid)` when either handle is invalid.
    /// - `Err(PushItemError::ParentFull)` when the parent is already at capacity.
    /// - `Err(PushItemError::MaxLayerExceeded)` when the maximum nesting depth would be exceeded.
    /// - `Err(PushItemError::IconViewChildMismatch)` when an icon-view parent receives a non-icon child.
    ///
    /// Behavior:
    /// - The child is stored by pointer; it is not copied.
    /// - Child layout metadata is initialized by the native runtime during the push.
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

    /// Replaces the renderer's current list icon pack.
    ///
    /// Parameters:
    /// - `icon_pack`: icon pack descriptor used by list rendering.
    pub fn set_icon_pack(&self, icon_pack: IconPack) {
        unsafe { raw::vision_ui_list_icon_set(self.raw_mut_ptr(), icon_pack) }
    }

    /// Returns the current list icon pack.
    ///
    /// Returns:
    /// - the icon pack currently active in the native renderer.
    pub fn icon_pack(&self) -> IconPack {
        unsafe { raw::vision_ui_list_icon_get_current(self.raw_ptr()) }
    }
}

/// Safe inline-owned Vision UI instance paired with a backend driver.
///
/// This is the main Rust entry point. It owns the native `vision_ui_t` value
/// inline and also owns the driver used by the C runtime.
pub struct VisionUi<D> {
    raw: raw::vision_ui_t,
    driver: D,
    driver_desc: raw::vision_ui_driver_t,
    driver_ops: raw::vision_ui_driver_ops_t,
    #[cfg(feature = "alloc")]
    toggle_closures: Vec<Box<ToggleClosure>>,
    #[cfg(feature = "alloc")]
    slide_closures: Vec<Box<SlideClosure>>,
    #[cfg(feature = "alloc")]
    scene_closures: Vec<Box<SceneClosureBindings>>,
}

impl<D: driver::Driver> VisionUi<D> {
    /// Creates a new inline-owned UI instance with an attached driver.
    ///
    /// Parameters:
    /// - `driver`: backend driver owned by the UI wrapper.
    ///
    /// Returns:
    /// - a new `VisionUi` value containing both the native UI state and the driver.
    ///
    /// Behavior:
    /// - Storage for the native `vision_ui_t` lives inline inside this Rust value.
    /// - Initialization uses `vision_ui_init(...)`.
    /// - The driver is rebound into the native UI automatically before runtime calls.
    /// - Dropping the wrapper later calls `vision_ui_destroy(...)` to release library-owned items.
    pub fn new(driver: D) -> Self {
        let mut raw = MaybeUninit::<raw::vision_ui_t>::uninit();
        unsafe { raw::vision_ui_init(raw.as_mut_ptr()) };
        let mut ui = Self {
            raw: unsafe { raw.assume_init() },
            driver,
            driver_desc: raw::vision_ui_driver_t {
                context: ptr::null_mut(),
                ops: ptr::null(),
            },
            driver_ops: driver_ops::<D>(),
            #[cfg(feature = "alloc")]
            toggle_closures: Vec::new(),
            #[cfg(feature = "alloc")]
            slide_closures: Vec::new(),
            #[cfg(feature = "alloc")]
            scene_closures: Vec::new(),
        };
        ui.sync_driver_binding();
        ui
    }

    /// Returns a callback-safe borrowed handle to the same UI instance.
    ///
    /// Returns:
    /// - a lightweight [`UiRef`] pointing at this inline-owned UI.
    pub fn ui(&self) -> UiRef {
        UiRef {
            raw: NonNull::from(&self.raw),
        }
    }

    /// Returns the raw native pointer for advanced interop.
    ///
    /// Returns:
    /// - a shared pointer to the inline `vision_ui_t`.
    pub fn raw_ptr(&self) -> *const raw::vision_ui_t {
        &self.raw as *const raw::vision_ui_t
    }

    /// Returns the raw mutable native pointer for advanced interop.
    ///
    /// Returns:
    /// - a mutable pointer to the inline `vision_ui_t`.
    pub fn raw_mut_ptr(&mut self) -> *mut raw::vision_ui_t {
        &mut self.raw as *mut raw::vision_ui_t
    }

    /// Returns a shared reference to the owned driver.
    pub fn driver(&self) -> &D {
        &self.driver
    }

    /// Returns a mutable reference to the owned driver.
    ///
    /// Behavior:
    /// - The native driver binding is refreshed before the mutable reference is returned.
    pub fn driver_mut(&mut self) -> &mut D {
        self.sync_driver_binding();
        &mut self.driver
    }

    fn sync_driver_binding(&mut self) {
        self.driver_desc.context = (&mut self.driver as *mut D).cast();
        self.driver_desc.ops = ptr::from_ref(&self.driver_ops);
        self.raw.driver = self.driver_desc;
    }

    /// Initializes selector, camera, and list runtime state after the tree is built.
    ///
    /// Returns:
    /// - `Ok(())` when runtime state was initialized successfully.
    /// - `Err(InitError::RootItemNotSet)` when no root item has been attached yet.
    ///
    /// Behavior:
    /// - Call this after the tree is built and the root has been attached.
    /// - This prepares selector, camera, and list runtime state before rendering begins.
    pub fn initialize_runtime(&mut self) -> Result<(), InitError> {
        self.sync_driver_binding();
        match unsafe { raw::vision_ui_core_init(self.raw_mut_ptr()) } {
            raw::vision_ui_core_init_result_t_VisionUiCoreInitOk => Ok(()),
            raw::vision_ui_core_init_result_t_VisionUiCoreInitRootItemNotSet => {
                Err(InitError::RootItemNotSet)
            }
            other => Err(InitError::Unknown(other)),
        }
    }

    /// Marks the UI as active and prepares the renderer to draw.
    ///
    /// Behavior:
    /// - This forwards to `vision_ui_render_init(...)`.
    /// - Call this after [`VisionUi::initialize_runtime`] and before the main render loop.
    pub fn initialize_rendering(&mut self) {
        self.sync_driver_binding();
        unsafe { raw::vision_ui_render_init(self.raw_mut_ptr()) }
    }

    /// Runs one frame of UI logic and rendering.
    ///
    /// Behavior:
    /// - This executes one iteration of the native UI loop, including input handling,
    ///   scene transitions, and drawing.
    /// - It forwards directly to `vision_ui_step_render(...)`.
    pub fn render_frame(&mut self) {
        self.sync_driver_binding();
        unsafe { raw::vision_ui_step_render(self.raw_mut_ptr()) }
    }

    /// Configures the optional startup logo shown before the main UI loop.
    ///
    /// Parameters:
    /// - `bitmap`: startup logo validated against the configured screen size.
    /// - `duration`: how long the logo should remain visible before normal UI rendering begins.
    ///
    /// Returns:
    /// - `Ok(())` when the logo length fits in Vision UI's `u32` byte-count range.
    /// - `Err(Error::BitmapTooLarge(_))` when the bitmap length exceeds that range.
    ///
    /// Behavior:
    /// - The bitmap bytes are borrowed, not copied.
    /// - The logo is shown by the native runtime before normal rendering takes over.
    pub fn set_startup_logo(
        &mut self,
        bitmap: StartupLogo,
        duration: Duration,
    ) -> Result<(), Error> {
        let bitmap = bitmap.bitmap();
        let span = duration_millis_u32(duration)?;
        unsafe { raw::vision_ui_start_logo_set(self.raw_mut_ptr(), bitmap.bytes().as_ptr(), span) };
        Ok(())
    }

    /// Returns whether the UI has completed its exit flow.
    ///
    /// Returns:
    /// - `true` when the native UI reports that it has exited.
    pub fn is_closed(&self) -> bool {
        self.ui().is_closed()
    }

    /// Returns whether background interaction should be paused behind the UI.
    ///
    /// Returns:
    /// - `true` when Vision UI wants background activity to remain frozen.
    pub fn freezes_background(&self) -> bool {
        self.ui().freezes_background()
    }

    /// Sets the small utility font.
    ///
    /// Parameters:
    /// - `font`: font descriptor forwarded to the native UI state.
    pub fn set_mini_font(&mut self, font: Font) {
        self.ui().set_mini_font(font)
    }

    /// Installs a custom allocator for subsequent library-managed allocations.
    ///
    /// Type parameters:
    /// - `A`: allocator implementation used for future constructor-created items and their destruction.
    ///
    /// Behavior:
    /// - This does not move or reallocate the `VisionUi` instance itself.
    /// - It affects subsequent native allocations owned by the UI runtime.
    pub fn set_allocator<A: Allocator>(&mut self) {
        unsafe { raw::vision_ui_allocator_set(self.raw_mut_ptr(), Some(allocator_trampoline::<A>)) }
    }

    /// Installs a raw native allocator callback.
    ///
    /// Prefer [`VisionUi::set_allocator`] unless you need direct ABI parity.
    ///
    /// Parameters:
    /// - `allocator`: raw allocator callback following the native C ABI.
    ///
    /// Behavior:
    /// - This replaces the current allocator hook used for subsequent library-managed allocations.
    pub unsafe fn set_allocator_raw(&mut self, allocator: raw::vision_ui_allocator_t) {
        unsafe { raw::vision_ui_allocator_set(self.raw_mut_ptr(), allocator) }
    }

    /// Restores the default allocator behavior.
    ///
    /// Behavior:
    /// - Future native allocations fall back to Vision UI's built-in allocator path.
    pub fn clear_allocator(&mut self) {
        unsafe { raw::vision_ui_allocator_set(self.raw_mut_ptr(), None) }
    }

    /// Sets the main body font.
    ///
    /// Parameters:
    /// - `font`: font descriptor used for standard content rows.
    pub fn set_body_font(&mut self, font: Font) {
        self.ui().set_body_font(font)
    }

    /// Sets the title font.
    ///
    /// Parameters:
    /// - `font`: font descriptor used for primary titles.
    pub fn set_title_font(&mut self, font: Font) {
        self.ui().set_title_font(font)
    }

    /// Sets the subtitle font.
    ///
    /// Parameters:
    /// - `font`: font descriptor used for subtitle text.
    pub fn set_subtitle_font(&mut self, font: Font) {
        self.ui().set_subtitle_font(font)
    }

    /// Returns the current small utility font.
    ///
    /// Returns:
    /// - the font currently used for compact utility text.
    pub fn mini_font(&self) -> Font {
        self.ui().mini_font()
    }

    /// Returns the current main body font.
    ///
    /// Returns:
    /// - the font currently used for normal content rows.
    pub fn body_font(&self) -> Font {
        self.ui().body_font()
    }

    /// Returns the current title font.
    ///
    /// Returns:
    /// - the font currently used for primary titles.
    pub fn title_font(&self) -> Font {
        self.ui().title_font()
    }

    /// Returns the current subtitle font.
    ///
    /// Returns:
    /// - the font currently used for subtitle text.
    pub fn subtitle_font(&self) -> Font {
        self.ui().subtitle_font()
    }

    /// Shows a temporary notification overlay for the requested duration.
    ///
    /// Parameters:
    /// - `message`: static text borrowed by the native notification system.
    /// - `duration`: how long the notification should remain visible.
    ///
    /// Returns:
    /// - `Ok(())` when the duration fits in Vision UI's `u16` millisecond range.
    /// - `Err(Error::DurationOverflow(_))` when the duration is too large to represent.
    pub fn notify(&mut self, message: Text, duration: Duration) -> Result<(), Error> {
        self.ui().notify(message, duration)
    }

    /// Shows a centered alert overlay for the requested duration.
    ///
    /// Parameters:
    /// - `message`: static text borrowed by the native alert system.
    /// - `duration`: how long the alert should remain visible.
    ///
    /// Returns:
    /// - `Ok(())` when the duration fits in Vision UI's `u16` millisecond range.
    /// - `Err(Error::DurationOverflow(_))` when the duration is too large to represent.
    pub fn alert(&mut self, message: Text, duration: Duration) -> Result<(), Error> {
        self.ui().alert(message, duration)
    }

    /// Creates a plain list container that can hold child items.
    ///
    /// Parameters:
    /// - `title`: static label stored by the native item.
    /// - `capacity`: maximum number of direct children this list can hold.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item or its child array could not be allocated.
    pub fn list(&mut self, title: Text, capacity: usize) -> Result<Item, Error> {
        wrap_item(unsafe {
            raw::vision_ui_list_item_new(self.raw_ptr(), capacity, false, title.as_ptr())
        })
    }

    /// Creates an icon-view list container that accepts icon items as children.
    ///
    /// Parameters:
    /// - `title`: static label stored by the native item.
    /// - `capacity`: maximum number of direct children this icon-view list can hold.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item or its child array could not be allocated.
    pub fn icon_list(&mut self, title: Text, capacity: usize) -> Result<Item, Error> {
        wrap_item(unsafe {
            raw::vision_ui_list_item_new(self.raw_ptr(), capacity, true, title.as_ptr())
        })
    }

    /// Creates a non-interactive title row.
    ///
    /// Parameters:
    /// - `text`: static title text stored by the native item.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
    pub fn title(&mut self, text: Text) -> Result<Item, Error> {
        wrap_item(unsafe { raw::vision_ui_list_title_item_new(self.raw_ptr(), text.as_ptr()) })
    }

    /// Creates a selectable icon card, optionally with child items.
    ///
    /// Parameters:
    /// - `title`: static title text stored by the native item.
    /// - `description`: optional static description text.
    /// - `icon`: optional monochrome bitmap validated against the configured icon size.
    /// - `capacity`: maximum number of direct children this icon item can hold.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item or its child array could not be allocated.
    /// - `Err(Error::BitmapDimensionsMismatch { .. })` when the bitmap size does not match the configured icon size.
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
                self.raw_ptr(),
                capacity,
                icon_ptr,
                title_ptr,
                description_ptr,
            )
        };
        wrap_item(raw)
    }

    /// Creates a switch row without a callback.
    ///
    /// Parameters:
    /// - `label`: static label text stored by the native item.
    /// - `initial`: initial switch state.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
    pub fn switch(&mut self, label: Text, initial: bool) -> Result<Item, Error> {
        let raw = unsafe {
            raw::vision_ui_list_switch_item_new(
                self.raw_ptr(),
                label.as_ptr(),
                initial,
                None,
                ptr::null_mut(),
            )
        };
        wrap_item(raw)
    }

    /// Creates a switch row with a static callback binding.
    ///
    /// Parameters:
    /// - `label`: static label text stored by the native item.
    /// - `initial`: initial switch state.
    /// - `binding`: static callback binding invoked when the value changes.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
    pub fn switch_with<T: 'static>(
        &mut self,
        label: Text,
        initial: bool,
        binding: &'static ToggleBinding<T>,
    ) -> Result<Item, Error> {
        let raw = unsafe {
            raw::vision_ui_list_switch_item_new(
                self.raw_ptr(),
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
    /// Creates a switch row with a closure callback.
    ///
    /// Parameters:
    /// - `label`: static label text stored by the native item.
    /// - `initial`: initial switch state.
    /// - `on_changed`: closure invoked when the switch value changes.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds and callback storage is retained successfully.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
    ///
    /// Behavior:
    /// - This convenience API requires the `alloc` feature.
    /// - The closure is retained inside the Rust wrapper for as long as the UI value lives.
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
                self.raw_ptr(),
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

    /// Creates a numeric slider row without a callback.
    ///
    /// Parameters:
    /// - `label`: static label text stored by the native item.
    /// - `initial`: initial slider value.
    /// - `step`: step size applied by the native UI when the slider changes.
    /// - `range`: inclusive minimum and maximum slider bounds.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
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
                self.raw_ptr(),
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

    /// Creates a numeric slider row with a static callback binding.
    ///
    /// Parameters:
    /// - `label`: static label text stored by the native item.
    /// - `initial`: initial slider value.
    /// - `step`: step size applied by the native UI when the slider changes.
    /// - `range`: inclusive minimum and maximum slider bounds.
    /// - `binding`: static callback binding invoked when the value changes.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
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
                self.raw_ptr(),
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
    /// Creates a numeric slider row with a closure callback.
    ///
    /// Parameters:
    /// - `label`: static label text stored by the native item.
    /// - `initial`: initial slider value.
    /// - `step`: step size applied by the native UI when the slider changes.
    /// - `range`: inclusive minimum and maximum slider bounds.
    /// - `on_changed`: closure invoked when the slider value changes.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds and callback storage is retained successfully.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
    ///
    /// Behavior:
    /// - This convenience API requires the `alloc` feature.
    /// - The closure is retained inside the Rust wrapper for as long as the UI value lives.
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
                self.raw_ptr(),
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

    /// Creates a custom full-screen scene item without callbacks.
    ///
    /// Parameters:
    /// - `label`: static label text stored by the native item.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
    pub fn scene(&mut self, label: Text) -> Result<Item, Error> {
        let raw = unsafe {
            raw::vision_ui_list_user_item_new(
                self.raw_ptr(),
                label.as_ptr(),
                None,
                None,
                None,
                ptr::null_mut(),
            )
        };
        wrap_item(raw)
    }

    /// Creates a custom full-screen scene item with static callbacks.
    ///
    /// Parameters:
    /// - `label`: static label text stored by the native item.
    /// - `bindings`: static callback set invoked when the scene becomes active, renders, and exits.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
    pub fn scene_with<T: 'static>(
        &mut self,
        label: Text,
        bindings: &'static SceneBindings<T>,
    ) -> Result<Item, Error> {
        let raw = unsafe {
            raw::vision_ui_list_user_item_new(
                self.raw_ptr(),
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
    /// Creates a custom full-screen scene item with closure callbacks.
    ///
    /// Parameters:
    /// - `label`: static label text stored by the native item.
    /// - `init`: optional closure invoked once when entering the scene.
    /// - `render`: optional closure invoked while the scene remains active.
    /// - `exit`: optional closure invoked once when leaving the scene.
    ///
    /// Returns:
    /// - `Ok(Item)` when allocation succeeds and callback storage is retained successfully.
    /// - `Err(Error::AllocationFailed(_))` when the native item could not be allocated.
    ///
    /// Behavior:
    /// - This convenience API requires the `alloc` feature.
    /// - Any provided closures are retained inside the Rust wrapper for as long as the UI value lives.
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
                self.raw_ptr(),
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

    /// Attaches the root list shown by the UI.
    ///
    /// Parameters:
    /// - `item`: list container to use as the top-level root.
    ///
    /// Returns:
    /// - `Ok(())` when the root was accepted.
    /// - `Err(SetRootError::ItemInvalid)` when the supplied handle is invalid.
    pub fn set_root(&mut self, item: Item) -> Result<(), SetRootError> {
        self.ui().set_root(item)
    }

    /// Returns the current root item, if one has been attached.
    ///
    /// Returns:
    /// - `Some(Item)` when a root item is attached.
    /// - `None` when no root item has been set yet.
    pub fn root(&self) -> Option<Item> {
        self.ui().root()
    }

    /// Appends a child item to a parent list or icon-view container.
    ///
    /// Parameters:
    /// - `parent`: destination list or icon-view item.
    /// - `child`: child item appended directly under `parent`.
    ///
    /// Returns:
    /// - `Ok(())` when the child was appended successfully.
    /// - `Err(PushItemError::ItemInvalid)` when either handle is invalid.
    /// - `Err(PushItemError::ParentFull)` when the parent is already at capacity.
    /// - `Err(PushItemError::MaxLayerExceeded)` when the maximum nesting depth would be exceeded.
    /// - `Err(PushItemError::IconViewChildMismatch)` when an icon-view parent receives a non-icon child.
    pub fn push(&mut self, parent: Item, child: Item) -> Result<(), PushItemError> {
        self.ui().push(parent, child)
    }

    /// Replaces the renderer's current list icon pack.
    ///
    /// Parameters:
    /// - `icon_pack`: icon pack descriptor used by list rendering.
    pub fn set_icon_pack(&mut self, icon_pack: IconPack) {
        self.ui().set_icon_pack(icon_pack)
    }

    /// Returns the current list icon pack.
    ///
    /// Returns:
    /// - the icon pack currently active in the native renderer.
    pub fn icon_pack(&self) -> IconPack {
        self.ui().icon_pack()
    }
}

impl<D> Drop for VisionUi<D> {
    fn drop(&mut self) {
        unsafe { raw::vision_ui_destroy(&mut self.raw as *mut raw::vision_ui_t) }
    }
}

const EMPTY_CSTR_BYTES: &[u8; 1] = b"\0";

#[inline]
fn driver_ops<D: driver::Driver>() -> raw::vision_ui_driver_ops_t {
    raw::vision_ui_driver_ops_t {
        action_get: Some(hosted_action_get::<D>),
        ticks_ms_get: Some(hosted_ticks_ms_get::<D>),
        delay: Some(hosted_delay::<D>),
        font_set: Some(hosted_font_set::<D>),
        font_get: Some(hosted_font_get::<D>),
        str_draw: Some(hosted_str_draw::<D>),
        str_utf8_draw: Some(hosted_str_utf8_draw::<D>),
        str_width_get: Some(hosted_str_width_get::<D>),
        str_utf8_width_get: Some(hosted_str_utf8_width_get::<D>),
        str_height_get: Some(hosted_str_height_get::<D>),
        font_mode_set: Some(hosted_font_mode_set::<D>),
        font_direction_set: Some(hosted_font_direction_set::<D>),
        pixel_draw: Some(hosted_pixel_draw::<D>),
        circle_draw: Some(hosted_circle_draw::<D>),
        disc_draw: Some(hosted_disc_draw::<D>),
        box_r_draw: Some(hosted_box_r_draw::<D>),
        box_draw: Some(hosted_box_draw::<D>),
        frame_draw: Some(hosted_frame_draw::<D>),
        frame_r_draw: Some(hosted_frame_r_draw::<D>),
        line_h_draw: Some(hosted_line_h_draw::<D>),
        line_v_draw: Some(hosted_line_v_draw::<D>),
        line_draw: Some(hosted_line_draw::<D>),
        line_h_dotted_draw: Some(hosted_line_h_dotted_draw::<D>),
        line_v_dotted_draw: Some(hosted_line_v_dotted_draw::<D>),
        bmp_draw: Some(hosted_bmp_draw::<D>),
        color_draw: Some(hosted_color_draw::<D>),
        clip_window_set: Some(hosted_clip_window_set::<D>),
        clip_window_reset: Some(hosted_clip_window_reset::<D>),
        buffer_clear: Some(hosted_buffer_clear::<D>),
        buffer_send: Some(hosted_buffer_send::<D>),
        buffer_area_send: Some(hosted_buffer_area_send::<D>),
        buffer_pointer_get: Some(hosted_buffer_pointer_get::<D>),
    }
}

#[inline]
unsafe fn driver_mut<'a, D>(state: *mut c_void) -> &'a mut D {
    unsafe { &mut *state.cast::<D>() }
}

#[inline]
unsafe fn driver_ref<'a, D>(state: *mut c_void) -> &'a D {
    unsafe { &*state.cast::<D>() }
}

#[inline]
unsafe fn cstr_from_ptr<'a>(value: *const c_char) -> &'a CStr {
    if value.is_null() {
        unsafe { CStr::from_bytes_with_nul_unchecked(EMPTY_CSTR_BYTES) }
    } else {
        unsafe { CStr::from_ptr(value) }
    }
}

#[inline]
unsafe fn bitmap_from_ptr<'a>(bit_map: *const u8, width: u16, height: u16) -> &'a [u8] {
    if bit_map.is_null() {
        &[]
    } else {
        unsafe { slice::from_raw_parts(bit_map, bitmap_len(width, height)) }
    }
}

#[inline]
fn action_into_raw(action: Action) -> raw::vision_ui_action_t {
    match action {
        Action::None => raw::vision_ui_action_t_UiActionNone,
        Action::Previous => raw::vision_ui_action_t_UiActionGoPrev,
        Action::Next => raw::vision_ui_action_t_UiActionGoNext,
        Action::Enter => raw::vision_ui_action_t_UiActionEnter,
        Action::Exit => raw::vision_ui_action_t_UiActionExit,
    }
}

unsafe extern "C" fn hosted_action_get<D: driver::Driver>(
    state: *mut c_void,
) -> raw::vision_ui_action_t {
    action_into_raw(driver::Input::action(unsafe { driver_mut::<D>(state) }))
}

unsafe extern "C" fn hosted_ticks_ms_get<D: driver::Driver>(state: *mut c_void) -> u32 {
    driver::Timing::ticks(unsafe { driver_mut::<D>(state) })
        .as_millis()
        .min(u128::from(u32::MAX)) as u32
}

unsafe extern "C" fn hosted_delay<D: driver::Driver>(state: *mut c_void, ms: u32) {
    driver::Timing::delay(
        unsafe { driver_mut::<D>(state) },
        Duration::from_millis(u64::from(ms)),
    );
}

unsafe extern "C" fn hosted_font_set<D: driver::Driver>(state: *mut c_void, font: Font) {
    driver::Text::set_font(unsafe { driver_mut::<D>(state) }, font);
}

unsafe extern "C" fn hosted_font_get<D: driver::Driver>(state: *mut c_void) -> Font {
    driver::Text::font(unsafe { driver_ref::<D>(state) })
}

unsafe extern "C" fn hosted_str_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    str_: *const c_char,
) {
    driver::Text::draw_text(unsafe { driver_mut::<D>(state) }, x, y, unsafe {
        cstr_from_ptr(str_)
    });
}

unsafe extern "C" fn hosted_str_utf8_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    str_: *const c_char,
) {
    driver::Text::draw_utf8(unsafe { driver_mut::<D>(state) }, x, y, unsafe {
        cstr_from_ptr(str_)
    });
}

unsafe extern "C" fn hosted_str_width_get<D: driver::Driver>(
    state: *mut c_void,
    str_: *const c_char,
) -> u16 {
    driver::Text::text_width(unsafe { driver_ref::<D>(state) }, unsafe {
        cstr_from_ptr(str_)
    })
}

unsafe extern "C" fn hosted_str_utf8_width_get<D: driver::Driver>(
    state: *mut c_void,
    str_: *const c_char,
) -> u16 {
    driver::Text::utf8_width(unsafe { driver_ref::<D>(state) }, unsafe {
        cstr_from_ptr(str_)
    })
}

unsafe extern "C" fn hosted_str_height_get<D: driver::Driver>(state: *mut c_void) -> u16 {
    driver::Text::text_height(unsafe { driver_ref::<D>(state) })
}

unsafe extern "C" fn hosted_pixel_draw<D: driver::Driver>(state: *mut c_void, x: u16, y: u16) {
    driver::Draw::pixel(unsafe { driver_mut::<D>(state) }, x, y);
}

unsafe extern "C" fn hosted_circle_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    r: u16,
) {
    driver::Draw::circle(unsafe { driver_mut::<D>(state) }, x, y, r);
}

unsafe extern "C" fn hosted_disc_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    r: u16,
) {
    driver::Draw::disc(unsafe { driver_mut::<D>(state) }, x, y, r);
}

unsafe extern "C" fn hosted_box_r_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    w: u16,
    h: u16,
    r: u16,
) {
    driver::Draw::fill_rounded_rect(unsafe { driver_mut::<D>(state) }, x, y, w, h, r);
}

unsafe extern "C" fn hosted_box_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    w: u16,
    h: u16,
) {
    driver::Draw::fill_rect(unsafe { driver_mut::<D>(state) }, x, y, w, h);
}

unsafe extern "C" fn hosted_frame_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    w: u16,
    h: u16,
) {
    driver::Draw::stroke_rect(unsafe { driver_mut::<D>(state) }, x, y, w, h);
}

unsafe extern "C" fn hosted_frame_r_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    w: u16,
    h: u16,
    r: u16,
) {
    driver::Draw::stroke_rounded_rect(unsafe { driver_mut::<D>(state) }, x, y, w, h, r);
}

unsafe extern "C" fn hosted_line_h_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    l: u16,
) {
    driver::Draw::hline(unsafe { driver_mut::<D>(state) }, x, y, l);
}

unsafe extern "C" fn hosted_line_v_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    h: u16,
) {
    driver::Draw::vline(unsafe { driver_mut::<D>(state) }, x, y, h);
}

unsafe extern "C" fn hosted_line_draw<D: driver::Driver>(
    state: *mut c_void,
    x1: u16,
    y1: u16,
    x2: u16,
    y2: u16,
) {
    driver::Draw::line(unsafe { driver_mut::<D>(state) }, x1, y1, x2, y2);
}

unsafe extern "C" fn hosted_line_h_dotted_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    l: u16,
) {
    driver::Draw::dotted_hline(unsafe { driver_mut::<D>(state) }, x, y, l);
}

unsafe extern "C" fn hosted_line_v_dotted_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    h: u16,
) {
    driver::Draw::dotted_vline(unsafe { driver_mut::<D>(state) }, x, y, h);
}

unsafe extern "C" fn hosted_bmp_draw<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    w: u16,
    h: u16,
    bit_map: *const u8,
) {
    driver::Draw::bitmap(unsafe { driver_mut::<D>(state) }, x, y, w, h, unsafe {
        bitmap_from_ptr(bit_map, w, h)
    });
}

unsafe extern "C" fn hosted_color_draw<D: driver::Driver>(state: *mut c_void, color: u8) {
    driver::Draw::set_color(unsafe { driver_mut::<D>(state) }, color);
}

unsafe extern "C" fn hosted_font_mode_set<D: driver::Driver>(state: *mut c_void, mode: u8) {
    driver::Text::set_font_mode(unsafe { driver_mut::<D>(state) }, mode);
}

unsafe extern "C" fn hosted_font_direction_set<D: driver::Driver>(state: *mut c_void, dir: u8) {
    driver::Text::set_font_direction(unsafe { driver_mut::<D>(state) }, dir);
}

unsafe extern "C" fn hosted_clip_window_set<D: driver::Driver>(
    state: *mut c_void,
    x0: i16,
    y0: i16,
    x1: i16,
    y1: i16,
) {
    driver::Draw::clip_rect(unsafe { driver_mut::<D>(state) }, x0, y0, x1, y1);
}

unsafe extern "C" fn hosted_clip_window_reset<D: driver::Driver>(state: *mut c_void) {
    driver::Draw::reset_clip(unsafe { driver_mut::<D>(state) });
}

unsafe extern "C" fn hosted_buffer_clear<D: driver::Driver>(state: *mut c_void) {
    driver::Buffer::clear(unsafe { driver_mut::<D>(state) });
}

unsafe extern "C" fn hosted_buffer_send<D: driver::Driver>(state: *mut c_void) {
    driver::Buffer::present(unsafe { driver_mut::<D>(state) });
}

unsafe extern "C" fn hosted_buffer_area_send<D: driver::Driver>(
    state: *mut c_void,
    x: u16,
    y: u16,
    w: u16,
    h: u16,
) {
    driver::Buffer::present_area(unsafe { driver_mut::<D>(state) }, x, y, w, h);
}

unsafe extern "C" fn hosted_buffer_pointer_get<D: driver::Driver>(
    state: *mut c_void,
) -> *mut c_void {
    driver::Buffer::buffer_ptr(unsafe { driver_ref::<D>(state) }).unwrap_or(ptr::null_mut())
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
