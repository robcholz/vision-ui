use core::ffi::{c_char, c_void, CStr};
use core::mem::MaybeUninit;
use core::time::Duration;
use thiserror::Error;
use vision_ui::{
    driver::{Buffer, Draw, Input, RawHandle, Text as DriverText, Timing},
    Action, Font, SCREEN_HEIGHT, SCREEN_WIDTH,
};

mod sim {
    #![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]
    include!(concat!(env!("OUT_DIR"), "/sim_bindings.rs"));
}

unsafe extern "C" {
    fn u8g_sdl_get_key() -> i32;
    fn SDL_GetTicks() -> u32;
    fn SDL_Delay(ms: u32);
    fn u8g2_SetFont(u8g2: *mut c_void, font: *const c_void);
    fn u8g2_DrawStr(u8g2: *mut c_void, x: u16, y: u16, text: *const c_char) -> u16;
    fn u8g2_DrawUTF8(u8g2: *mut c_void, x: u16, y: u16, text: *const c_char) -> u16;
    fn u8g2_GetStrWidth(u8g2: *mut c_void, text: *const c_char) -> u16;
    fn u8g2_GetUTF8Width(u8g2: *mut c_void, text: *const c_char) -> u16;
    fn u8g2_GetMaxCharHeight(u8g2: *mut c_void) -> i16;
    fn u8g2_SetFontMode(u8g2: *mut c_void, mode: u8);
    fn u8g2_SetFontDirection(u8g2: *mut c_void, direction: u8);
    fn u8g2_DrawPixel(u8g2: *mut c_void, x: u16, y: u16);
    fn u8g2_DrawCircle(u8g2: *mut c_void, x: u16, y: u16, r: u16, option: u8);
    fn u8g2_DrawDisc(u8g2: *mut c_void, x: u16, y: u16, r: u16, option: u8);
    fn u8g2_DrawRBox(u8g2: *mut c_void, x: u16, y: u16, w: u16, h: u16, r: u16);
    fn u8g2_DrawBox(u8g2: *mut c_void, x: u16, y: u16, w: u16, h: u16);
    fn u8g2_DrawFrame(u8g2: *mut c_void, x: u16, y: u16, w: u16, h: u16);
    fn u8g2_DrawRFrame(u8g2: *mut c_void, x: u16, y: u16, w: u16, h: u16, r: u16);
    fn u8g2_DrawHLine(u8g2: *mut c_void, x: u16, y: u16, len: u16);
    fn u8g2_DrawVLine(u8g2: *mut c_void, x: u16, y: u16, len: u16);
    fn u8g2_DrawLine(u8g2: *mut c_void, x1: u16, y1: u16, x2: u16, y2: u16);
    fn u8g2_DrawXBM(u8g2: *mut c_void, x: u16, y: u16, w: u16, h: u16, bitmap: *const u8);
    fn u8g2_SetDrawColor(u8g2: *mut c_void, color: u8);
    fn u8g2_SetClipWindow(u8g2: *mut c_void, x0: i16, y0: i16, x1: i16, y1: i16);
    fn u8g2_SetMaxClipWindow(u8g2: *mut c_void);
    fn u8g2_ClearBuffer(u8g2: *mut c_void);
    fn u8g2_SendBuffer(u8g2: *mut c_void);
    fn u8g2_UpdateDisplayArea(u8g2: *mut c_void, tx: u8, ty: u8, tw: u8, th: u8);
    static mut U8G2_BUFFER: [u8; BUFFER_SIZE];
    static u8g2_font_fub42_tf: [u8; 0];
}

const BUFFER_SIZE: usize = (SCREEN_WIDTH as usize * SCREEN_HEIGHT as usize) / 8;
const U8G2_DRAW_ALL: u8 = 0x0F;
const KEY_SPACE: i32 = b' ' as i32;
const KEY_Q: i32 = b'q' as i32;

static MY_FONT: [u8; 2372] = *b"\x92\x00\x03\x02\x04\x04\x04\x04\x05\x0b\x0d\x00\xfe\x08\xfe\x0a\xff\x01d\x02\xd9\x04$ \x05\x00\x98\x16!\x07\x91\x8a\
\x16\xa7\x00\"\x074\xf9\x16\x91)#\x0e\x96\x887Q\xcb\xb0D\xbd\x0cK\xd4\x02$\x0f\xa5xV\xd9RQ\x62QK\x94\xca\x16\x01%\x10\x96x\x66Q\xd2EK\xc2\x64\x89\x92.Z\
\x02&\x0e\x85\x88VY\x12%YeJ\xa2H\x09'\x061\xfa\x15\x03(\x0b\xb3yVI\x94D\xdd\xa2,)\x0c\xb3y\x16Y\x94E]\xa2$\x02*\x0cu\x88V\x95\xcaAY\x9aB\x00+\x0bw\x88xqm\x18\xb2\
\xb8\x06,\x072z\x16\x8a\x02-\x07\x15\xc8\x16\x83\x00.\x06!\x8a\x15\x02/\x0c\xc4xvMY)+e5\x000\x0a\x85\x88\x36K\xe6[\xb2\x001\x09\x85\x88\x16c\x9f\x06\x012\x0b\x85\x88\x36K\x16\xd6\
\xda\x06\x013\x0d\x85\x88\x16C\x1a&k\x18\x0e\x0a\x004\x0e\x86\x88wZ\x12u\xc9\x92aL\x13\x005\x0d\x85\x88\x16C\x12\x86C\x1a\x86\x83\x026\x0d\x85\x88\x36K\x18\x0eIfK\x16\x007\x0b\x85\x88\x16\
\x83X\x0b\xb3\xb0\x048\x0d\x85\x88\x36K\xa6%KfK\x16\x009\x0d\x85\x88\x36KfK\x86PK\x16\x00:\x07a\x88\x12\x92\x00;\x09\x82z\x16C\xac(\x00<\x08\x95\x88\x96Y\xd7\x0e=\x085\xb8\x16\
\x83:\x08>\x09\x95\x88\x16i\xb7\x8e\x00?\x0d\x95\x88\x36K\xa6\x85\x91\x96Ca\x04@\x14\xa7xX[%\x91\xb4DR\x94\x8aR\x91\x12%\xcb&\x00A\x0e\x87\x88xq\x9a\x84IV\x1a\x94T\x0dB\x0d\x86\
\x88\x17\x83\x12\x8a\xc3\x12\x1a\x87\x05C\x0c\x86\x887C\x12\xaa\x1d\x93!\x01D\x0d\x87\x88\x18\x83\x14&\xa9\xc7d\x90\x00E\x0b\x85\x88\x16\xc7p\x18\x8b\x83\x00F\x0b\x85\x88\x16\xc7pH\xc2F\x00G\x0d\
\x86\x887C\x12\xaa\xb5QL\x06\x01H\x0b\x86\x88\x17\xa1q\x18D\xc7\x00I\x09\x83\x88\x14K\xd4\xcb\x00J\x08\xa3hT\xfdi\x01K\x0e\x85\x88\x16\x99\x94\x944-\x89*Y\x00L\x09\x85\x88\x16a\x1f\x07\x01\
M\x10\x87\x88\x18\xda\x90-\x15\xa5\"ER\xa4\xaa\x01N\x0d\x86\x88\x17\xe1\xa6DR\xa2\x8d\xc6\x00O\x0c\x87\x88X[%uM\xb2l\x02P\x0c\x85\x88\x16C\x92\xd9\x06%,\x02Q\x0d\x97xX[%uM\
\xb2lO\x00R\x10\x86\x88\x17C\x94%Y\x92%C\xd4\x96\x84\x01S\x0c\x85\x88\x36\x83\x98\xaea8(\x00T\x0a\x87\x88\x18\x87,\xee\x1b\x00U\x0a\x86\x88\x17\xa1\x1f\x93!\x01V\x0f\x87\x88\x18\xa9\x9adQV\
\x09\x934\xce\x00W\x11\x89\x88\x1aY\xa6eZ\xa5S\xd2\x94\xb4\x15\xb3\x08X\x0e\x86\x88\x17\xa1\x98D\x99\x16\xb5\x84b\x00Y\x0c\x87\x88\x18i\x92U\xd2\xb8\x1b\x00Z\x0a\x87\x88\x18\x87\xb4\xcf\xc3\x10[\x09\xb3z\
\x16C\xd4\x9f\x06\\\x0d\xa5x\x16a\x1a\xa6a\x1a\xa6a\x00]\x09\xb3y\x16S\x7f\x1a\x02^\x085\xe8VYR\x0b_\x07\x15x\x16\x83\x00`\x072\xf9\x15I\x14a\x0ce\x88\x36K\x96\x0c\x9a\x96\x0c\x01b\
\x0c\x85\x88\x16a8$\x99\xdb\xa0\x00c\x08d\x885C\xd68d\x0b\x85\x88\x96\x95AsK\x86\x00e\x0be\x88\x36K6\x0ca:\x04f\x09\x83\x884\xd3\x10u\x02g\x0c\x85h\x36\x83\xe6\x96\x0ca\xb2\x00h\
\x0b\x85\x88\x16a8$\x99\xb7\x00i\x07\x81\x88\x12\xc90j\x09\xa2h\x63Y\xd2\xcb\x00k\x0d\x85\x88\x16a))iIT\xc9\x02l\x07\x81\x88\x12\x07\x01m\x0eg\x88\x18\x8b\x12ER$ER$\x15n\x09\
e\x88\x16C\x92y\x0bo\x0ae\x88\x36K\xe6\x96,\x00p\x0c\x85h\x16C\x92\xb9\x0dJ\x18\x02q\x0b\x85h\x36\x83\xe6\x96\x0ca\x01r\x08c\x88\x14C\xd4\x09s\x0ce\x88\x36K\x96\xa8I\x96,\x00t\x0a\
\x83\x88\x14Q4Dm\x02u\x09e\x88\x16\x99\xb7d\x08v\x0ce\x88\x16\x99\x96\x94\x92,\x8c\x00w\x0fg\x88\x18Q$EJE\xe9\x16e\x09\x00x\x0be\x88\x16YR\xab\xd4\xb4\x00y\x0d\x85h\x16\x99\
\x96\x94\x92,\xcc\xc2\x0cz\x0ae\x88\x16\x83\x98\xb5\x0d\x02{\x0b\xa3xTIT\xc9\xa2\xb6\x00|\x07\xb1z\x15\x0f\x02}\x0b\xa3x\x14YTK\xa2\x96\x08~\x07&\xb87\x92\x05\x80\x0f\x95\x88VR%\x1b\xa2\
l\x88\xc2(R\x00\x00\x00\x00\x04\xff\xff0\x0c\x0a\x84\xaf\x1c\x83\xd67\x000\x0d\x09\x84y|}\x1b\x04N2\x17\xb9y\x9c\xe90DM\xc3\x90\x86\xc3!\xcb\xb4l8\x849\x12\x02N\x8e\x17\xbbx<\xc39\
\xc7r,\x1d\x0ej\x8e\xe5X\x8e\xe5X\x8e\xac\x00Qe\x17\xbbx|:\x98c9\x96CI\x8e\xd4\xb38Kky\x92C\x01Qs\x16\xbbx\\i-\x1c\xce9\x96\x0e\x878\x87\x92<KK\xba\x00R\x07\
\x1b\xbbx\\9\x16\rS\x18ES4\x84Q1\xaaD%)\x8b\xb2jXS\x00R\xa8\x1c\xbbx\xfc\xe1\x10\xe5X\x0e\r\x07%*F\xc5(\x89J\xc3\x92\x85I\x96F\x0a\x00S\xcd\x1a\xbbx\xfc\xdb \xe7X\
\x8e\r\x83\x96da\x94\xa4Y\x9a%a\x94E\x89*S\xd1\x19\xbbx\\\x8dQ\x16e\xf1p\x08sl\x90\xb34\xa9fi\x96H\x91(S\xe3\x0c\x99\x89\x1c\x0f\xb9\xef\xc3A\x0fT/\x1a\xbax\xbc\xf10Hq\
\x14G\xc3 \xe5\xd00HjRM\x92a\x90\xd2\x00V\xfe\x19\xbay\x1c\x0fR*\r\x8a)\x9a4\xa94$\xd1\x10fZ(f\xc3AY\x16\x1b\xbbx\\a\x1a\xa6C\x92FI\x98%R\x12%E\xa9\x18\
\xc5Y\x1aV3\x00\\O\x19\xbbx\\\xc3)\x8f\x86S\x12fQq\x085\x0e\xa7\xb6(\x8b\x8a\x11\x00^U\x1c\xbbx\x1c\x0fZu\x18\xc24\x1c\x860\xc9\xa2\xe1\xa0%Q4\x1c\xa4\xc6(\x91\x00_\x00\x17\
\xbbx<\xc31\x8b\xb38\x8b\xb3l8h\xd50\r\xabY\x9c\x01_\x0f\x19\xbbx\xdcI\x8eDy8\x1c\xe4tH\xf2(\xcf\xe2,\xca\xb4d\x88\x05_\xc3\x18\xbbx\xbc9\x98Gy\x94\xa7Q\x12'\xe5\xa4\
\x98Ha\x1a\xc6\x83\x04c\x09\x1c\xbbx\\aZ\x1a\x0eR%\xaaD\xa92,Z\x94\x94\xa4,\x14\xb3$Z4\x01cb\x1e\xbbx\\Y\x9c\r\xc90\x85\xd1\xa0%JMiR\x86A\x11\xd3,\x11\xa3,\
Y\xd2\x00cn\x1d\xbbx\\\xd10\x15\x87!\x8c\xa2aJ\xa4P\x19\x16\xa9%\x1a\xa6dS\xda\x92\xd2 c\xa5\x1c\xbbx\\a\x9a\x0c\x07%k\x09\x93a\x88\xb4P\x19\x06%\x8b\xb2H\x09+\xc9-ep\
 \xbbx\x1cQRL\xa4lX\x86\xc8\x12%RRJZ\x86\xa5\x96DI\xa4di\x96$\xb7\x00e\xe0\x18\xbbx<\xc39\xc7r,\x1d\x0eb\x92#\xf5(\x8f\xb2(\xcb\xd6!e\xf6\x1d\xbbx\xfcH4\
\x84Q4\x9c\xc2\xa8\x12EC\x94D-Q1*FC\x98#\x13\x00f>\x1c\xbbx\\\xc3\x10\xa6\xe10\x84i8\x0cq\x92FI\x94%\xa5,iN\xc2\xe1 g:\x1c\xbbx\\9\x16\raT\x1a\xa6\
0\xaa-Q\xa6H\x95\xa8%*&Y\x12\x89\x03g\x7f\x1d\xbbx\\\xa9\x16m\xc3\x1e\xe5\xd1 -Y\xa4LI)\x11\x93,L\xa2$S\xb4\x00h7\x1b\xbbx\\\xb5\x96h8Ha\xb8\x0c\x92\x98%a\
\x9a\x0cC\x14\xa6a\x1af\x00po\x18\xbbx|9\x96\x0cKS\x96h\x9550*\xa6a\x1aV\xb2\xa6\xb2\x04ri\x1e\xbbx\\Y\x98da\x12\r\x07\xa5\xa2dIIJ*RE)\xb5\x95*Q%S\
\x00t\x06\x1a\xbbx\x9c\xc3S\xcb0DI\xd4\x12E\xc3S\x98F\x83\xa6ej>\x0c\x01u1\x16\xb9y\x9c9\x12\x0e\x87,\xd3\xb2\xe1\xa0eZ\xa6e\xc3A\x0fy:\x14\xabx<\xc3\x9di8\xa8yT\
\x8c\xb2(\x0b\xc5\xa2\x0cy\xd2\x1b\xbbx|\xb51\x8d\x92\xca\x90\x94Z\xa2d)*Q\xa4\xc4Y\x1afj\xa2\x02~\xbf\x1d\xbbx\\Y\x12fQq\x88\xa2q\x1b\xa4d\r\xa3d\x88\x92\x8a*II2i\
\x02~\xe7\x1a\xbbx\\\x8d\x91\xd2\x96LI):Lm\x95C\xa2T\xa3P\x89\"m\x18~\xed\x1b\xbbx\\a\x1a\rR\x1a&\xc9p\xaaDI[)Z\x86!O\xa5\xc4$\x06\x7fn\x1a\xbbx<\xc3\
\xa9\x12E\xc3\x79\x1d\x0eR\x1a\x0eC\x98\x86\xc3\x10\xa6\xd1p\x10\x8b\xbe\x19\xbbx<\xd9\x10F\xf5(\xce\x86\x1dK\x86-\xcaZB-\xcd\x92t\x13\x8d\xf3\"\xbbx\x1cC\x94D\x91R\x91\x12\xcb\x10%a\
\x96\x84\x91\xa2$KE\xc9\x92(Y*[\x94\x84\xa1\x00\x8fl\x1b\xbbx\\a6$\x83\x94\x86\xca0(Y8$\x83\x16gKiL\x8bi\x04\x8f\xdb\x1a\xbbx<Y\x14F\xc5d\x98\xa3h*\x0e\xa7\xc6\
\xa8\x98d\x9550\xca\x86!\x90 \x1c\xbbx<i)I\xa3A\x8d\xb2e\x18\xa2\x1c\x8b\x06-\xcaJ\x83\x94\xe4\xd90\x04\x90\xe8!\xbbx|\xd9\xf0\x94dIT\x89\x92d\xb8CI4(Q\x92%Q\x92%J\
\x82(\xa5,\xc9\x00\x95.\x1f\xbbx<\xc9\x94e\xc3\xa2d\x89\x96\x0c\x07)\xa9\r\xa7Z\x92\x0cK\x94e\x8a\x94%\xd1 \x95\x7f\x16\xbay\\a-\x8d\xe2$\x97\x87C\x94\xe4Q\x9c\xa5I\x14\x1a\x95\xf4\
\x19\xbay<\xd10\xc5:B\r\x91T\x92\x86H*I%i\x88tD\x1e\x9aq \xbbx\x1cC4hQ\x9aDYR\x92\x92RR\x19\x92(M\xa2d\xe9\x98Ha\x12K\x83\x00\x00";

#[derive(Debug, Error, Clone, Copy, PartialEq, Eq)]
pub enum Simulator240x240Error {
    #[error("failed to allocate the simulator backend")]
    AllocationFailed,
}

pub struct Simulator240x240 {
    storage: Box<MaybeUninit<sim::u8g2_t>>,
    current_font: Font,
}

impl Simulator240x240 {
    pub fn new() -> Result<Self, Simulator240x240Error> {
        let mut storage = Box::new(MaybeUninit::<sim::u8g2_t>::uninit());
        let raw = storage.as_mut_ptr();

        unsafe {
            sim::u8x8_setup_sdl_240x240(raw.cast::<sim::u8x8_t>());
            sim::u8g2_SetupBuffer(
                raw,
                core::ptr::addr_of_mut!(U8G2_BUFFER).cast(),
                (SCREEN_HEIGHT / 8) as u8,
                Some(sim::u8g2_ll_hvline_vertical_top_lsb),
                &raw const sim::u8g2_cb_r0,
            );
            sim::u8x8_InitDisplay(raw.cast::<sim::u8x8_t>());
            sim::u8x8_SetPowerSave(raw.cast::<sim::u8x8_t>(), 0);
        }

        Ok(Self {
            storage,
            current_font: Font {
                font: core::ptr::null(),
                top_compensation: 0,
                bottom_compensation: 0,
            },
        })
    }

    pub fn title_font() -> Font {
        Font {
            font: (&raw const u8g2_font_fub42_tf).cast(),
            top_compensation: -2,
            bottom_compensation: 18,
        }
    }

    pub fn body_font() -> Font {
        Font {
            font: (&raw const MY_FONT).cast(),
            top_compensation: 0,
            bottom_compensation: 0,
        }
    }

    pub fn subtitle_font() -> Font {
        Font {
            font: (&raw const MY_FONT).cast(),
            top_compensation: 0,
            bottom_compensation: 3,
        }
    }

    fn raw_handle_const(&self) -> *mut c_void {
        self.storage.as_ptr().cast::<sim::u8g2_t>() as *mut c_void
    }

    fn adjusted_baseline(&self, y: u16) -> u16 {
        let adjusted = i32::from(y) - i32::from(self.current_font.bottom_compensation);
        adjusted.clamp(0, i32::from(u16::MAX)) as u16
    }
}

impl RawHandle for Simulator240x240 {
    fn as_raw_handle(&mut self) -> *mut c_void {
        self.storage.as_mut_ptr().cast()
    }
}

impl Input for Simulator240x240 {
    fn action(&mut self) -> Action {
        match unsafe { u8g_sdl_get_key() } {
            273 => Action::Previous,
            274 => Action::Next,
            KEY_SPACE => Action::Enter,
            KEY_Q => Action::Exit,
            _ => Action::None,
        }
    }
}

impl Timing for Simulator240x240 {
    fn ticks(&mut self) -> Duration {
        Duration::from_millis(u64::from(unsafe { SDL_GetTicks() }))
    }

    fn delay(&mut self, duration: Duration) {
        let ms = duration.as_millis().min(u128::from(u32::MAX)) as u32;
        unsafe { SDL_Delay(ms) }
    }
}

impl DriverText for Simulator240x240 {
    fn set_font(&mut self, font: Font) {
        unsafe { u8g2_SetFont(self.as_raw_handle(), font.font) }
        self.current_font = font;
    }

    fn font(&self) -> Font {
        self.current_font
    }

    fn draw_text(&mut self, x: u16, y: u16, text: &CStr) {
        unsafe { u8g2_DrawStr(self.as_raw_handle(), x, self.adjusted_baseline(y), text.as_ptr()) };
    }

    fn draw_utf8(&mut self, x: u16, y: u16, text: &CStr) {
        unsafe { u8g2_DrawUTF8(self.as_raw_handle(), x, self.adjusted_baseline(y), text.as_ptr()) };
    }

    fn text_width(&self, text: &CStr) -> u16 {
        unsafe { u8g2_GetStrWidth(self.raw_handle_const(), text.as_ptr()) }
    }

    fn utf8_width(&self, text: &CStr) -> u16 {
        unsafe { u8g2_GetUTF8Width(self.raw_handle_const(), text.as_ptr()) }
    }

    fn text_height(&self) -> u16 {
        let height = i32::from(unsafe { u8g2_GetMaxCharHeight(self.raw_handle_const()) });
        let adjusted = height + i32::from(self.current_font.top_compensation);
        adjusted.clamp(0, i32::from(u16::MAX)) as u16
    }

    fn set_font_mode(&mut self, mode: u8) {
        unsafe { u8g2_SetFontMode(self.as_raw_handle(), if mode == 0 { 0 } else { 1 }) }
    }

    fn set_font_direction(&mut self, direction: u8) {
        unsafe { u8g2_SetFontDirection(self.as_raw_handle(), direction & 0x03) }
    }
}

impl Draw for Simulator240x240 {
    fn pixel(&mut self, x: u16, y: u16) {
        unsafe { u8g2_DrawPixel(self.as_raw_handle(), x, y) }
    }

    fn circle(&mut self, x: u16, y: u16, radius: u16) {
        unsafe { u8g2_DrawCircle(self.as_raw_handle(), x, y, radius, U8G2_DRAW_ALL) }
    }

    fn disc(&mut self, x: u16, y: u16, radius: u16) {
        unsafe { u8g2_DrawDisc(self.as_raw_handle(), x, y, radius, U8G2_DRAW_ALL) }
    }

    fn fill_rounded_rect(&mut self, x: u16, y: u16, width: u16, height: u16, radius: u16) {
        unsafe { u8g2_DrawRBox(self.as_raw_handle(), x, y, width, height, radius) }
    }

    fn fill_rect(&mut self, x: u16, y: u16, width: u16, height: u16) {
        unsafe { u8g2_DrawBox(self.as_raw_handle(), x, y, width, height) }
    }

    fn stroke_rect(&mut self, x: u16, y: u16, width: u16, height: u16) {
        unsafe { u8g2_DrawFrame(self.as_raw_handle(), x, y, width, height) }
    }

    fn stroke_rounded_rect(&mut self, x: u16, y: u16, width: u16, height: u16, radius: u16) {
        unsafe { u8g2_DrawRFrame(self.as_raw_handle(), x, y, width, height, radius) }
    }

    fn hline(&mut self, x: u16, y: u16, length: u16) {
        unsafe { u8g2_DrawHLine(self.as_raw_handle(), x, y, length) }
    }

    fn vline(&mut self, x: u16, y: u16, height: u16) {
        unsafe { u8g2_DrawVLine(self.as_raw_handle(), x, y, height) }
    }

    fn line(&mut self, x1: u16, y1: u16, x2: u16, y2: u16) {
        unsafe { u8g2_DrawLine(self.as_raw_handle(), x1, y1, x2, y2) }
    }

    fn dotted_hline(&mut self, x: u16, y: u16, length: u16) {
        let mut i = 0;
        while i < length {
            unsafe { u8g2_DrawPixel(self.as_raw_handle(), x.saturating_add(i), y) }
            i = i.saturating_add(2);
        }
    }

    fn dotted_vline(&mut self, x: u16, y: u16, height: u16) {
        let mut i = 0;
        while i < height {
            unsafe { u8g2_DrawPixel(self.as_raw_handle(), x, y.saturating_add(i)) }
            i = i.saturating_add(2);
        }
    }

    fn bitmap(&mut self, x: u16, y: u16, width: u16, height: u16, bitmap: &[u8]) {
        unsafe { u8g2_DrawXBM(self.as_raw_handle(), x, y, width, height, bitmap.as_ptr()) }
    }

    fn set_color(&mut self, color: u8) {
        unsafe { u8g2_SetDrawColor(self.as_raw_handle(), color) }
    }

    fn clip_rect(&mut self, x0: i16, y0: i16, x1: i16, y1: i16) {
        unsafe { u8g2_SetClipWindow(self.as_raw_handle(), x0, y0, x1, y1) }
    }

    fn reset_clip(&mut self) {
        unsafe { u8g2_SetMaxClipWindow(self.as_raw_handle()) }
    }
}

impl Buffer for Simulator240x240 {
    fn clear(&mut self) {
        unsafe { u8g2_ClearBuffer(self.as_raw_handle()) }
    }

    fn present(&mut self) {
        unsafe { u8g2_SendBuffer(self.as_raw_handle()) }
    }

    fn present_area(&mut self, x: u16, y: u16, width: u16, height: u16) {
        if width == 0 || height == 0 {
            return;
        }

        let x_end = x.saturating_add(width).saturating_sub(1);
        let y_end = y.saturating_add(height).saturating_sub(1);
        let tx = (x / 8) as u8;
        let ty = (y / 8) as u8;
        let tw = ((x_end / 8).saturating_sub(x / 8).saturating_add(1)) as u8;
        let th = ((y_end / 8).saturating_sub(y / 8).saturating_add(1)) as u8;
        unsafe { u8g2_UpdateDisplayArea(self.as_raw_handle(), tx, ty, tw, th) }
    }

    fn buffer_ptr(&self) -> Option<*mut c_void> {
        Some(core::ptr::addr_of_mut!(U8G2_BUFFER).cast())
    }
}
