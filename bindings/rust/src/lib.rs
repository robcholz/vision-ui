use std::error::Error;
use std::ffi::{CString, NulError};
use std::fmt::{Display, Formatter};
use std::os::raw::c_char;
use std::sync::{Mutex, OnceLock};

unsafe extern "C" {
    static mut IS_IN_VISION_UI: bool;

    fn vision_ui_render_init();
    fn vision_ui_core_init();
    fn vision_ui_start_logo_set(bmp: *const u8, span: u32);
    fn vision_ui_step_render();
    fn vision_ui_is_exited() -> bool;
    fn vision_ui_is_background_frozen() -> bool;

    fn vision_ui_notification_push(content: *const c_char, span: u16);
    fn vision_ui_alert_push(content: *const c_char, span: u16);
}

#[derive(Debug)]
pub enum VisionUiError {
    NulByteInString(NulError),
    BitmapTooLarge(usize),
}

impl Display for VisionUiError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::NulByteInString(_) => write!(f, "string contains an interior NUL byte"),
            Self::BitmapTooLarge(size) => {
                write!(f, "bitmap length {size} exceeds u32::MAX")
            }
        }
    }
}

impl Error for VisionUiError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            Self::NulByteInString(err) => Some(err),
            Self::BitmapTooLarge(_) => None,
        }
    }
}

impl From<NulError> for VisionUiError {
    fn from(value: NulError) -> Self {
        Self::NulByteInString(value)
    }
}

fn retained_messages() -> &'static Mutex<Vec<CString>> {
    static RETAINED_MESSAGES: OnceLock<Mutex<Vec<CString>>> = OnceLock::new();
    RETAINED_MESSAGES.get_or_init(|| Mutex::new(Vec::new()))
}

fn retained_logo() -> &'static Mutex<Option<Box<[u8]>>> {
    static RETAINED_LOGO: OnceLock<Mutex<Option<Box<[u8]>>>> = OnceLock::new();
    RETAINED_LOGO.get_or_init(|| Mutex::new(None))
}

pub fn render_init() {
    unsafe { vision_ui_render_init() }
}

pub fn core_init() {
    unsafe { vision_ui_core_init() }
}

pub fn set_start_logo(bitmap: &[u8]) -> Result<(), VisionUiError> {
    let span = u32::try_from(bitmap.len()).map_err(|_| VisionUiError::BitmapTooLarge(bitmap.len()))?;
    let owned = bitmap.to_vec().into_boxed_slice();
    let ptr = owned.as_ptr();

    {
        let mut slot = retained_logo().lock().expect("logo mutex poisoned");
        *slot = Some(owned);
    }

    unsafe {
        vision_ui_start_logo_set(ptr, span);
    }
    Ok(())
}

pub fn step_render() {
    unsafe { vision_ui_step_render() }
}

pub fn is_exited() -> bool {
    unsafe { vision_ui_is_exited() }
}

pub fn is_background_frozen() -> bool {
    unsafe { vision_ui_is_background_frozen() }
}

pub fn push_notification(content: &str, span: u16) -> Result<(), VisionUiError> {
    let c_content = CString::new(content)?;
    let ptr = c_content.as_ptr();
    retained_messages()
        .lock()
        .expect("message mutex poisoned")
        .push(c_content);

    unsafe {
        vision_ui_notification_push(ptr, span);
    }
    Ok(())
}

pub fn push_alert(content: &str, span: u16) -> Result<(), VisionUiError> {
    let c_content = CString::new(content)?;
    let ptr = c_content.as_ptr();
    retained_messages()
        .lock()
        .expect("message mutex poisoned")
        .push(c_content);

    unsafe {
        vision_ui_alert_push(ptr, span);
    }
    Ok(())
}

pub fn is_in_vision_ui() -> bool {
    unsafe { IS_IN_VISION_UI }
}
