use crate::{Action, Font};
use std::ffi::{c_void, CStr};
use std::time::Duration;

pub trait Input {
    fn action(&mut self) -> Action;
}

pub trait Timing {
    fn ticks(&mut self) -> Duration;
    fn delay(&mut self, duration: Duration);
}

pub trait Text {
    fn set_font(&mut self, font: Font);
    fn font(&self) -> Font;
    fn draw_text(&mut self, x: u16, y: u16, text: &CStr);
    fn draw_utf8(&mut self, x: u16, y: u16, text: &CStr);
    fn text_width(&self, text: &CStr) -> u16;
    fn utf8_width(&self, text: &CStr) -> u16;
    fn text_height(&self) -> u16;
    fn set_font_mode(&mut self, mode: u8);
    fn set_font_direction(&mut self, direction: u8);
}

pub trait Draw {
    fn pixel(&mut self, x: u16, y: u16);
    fn circle(&mut self, x: u16, y: u16, radius: u16);
    fn disc(&mut self, x: u16, y: u16, radius: u16);
    fn fill_rounded_rect(&mut self, x: u16, y: u16, width: u16, height: u16, radius: u16);
    fn fill_rect(&mut self, x: u16, y: u16, width: u16, height: u16);
    fn stroke_rect(&mut self, x: u16, y: u16, width: u16, height: u16);
    fn stroke_rounded_rect(&mut self, x: u16, y: u16, width: u16, height: u16, radius: u16);
    fn hline(&mut self, x: u16, y: u16, length: u16);
    fn vline(&mut self, x: u16, y: u16, height: u16);
    fn line(&mut self, x1: u16, y1: u16, x2: u16, y2: u16);
    fn dotted_hline(&mut self, x: u16, y: u16, length: u16);
    fn dotted_vline(&mut self, x: u16, y: u16, height: u16);
    fn bitmap(&mut self, x: u16, y: u16, width: u16, height: u16, bitmap: &[u8]);
    fn set_color(&mut self, color: u8);
    fn clip_rect(&mut self, x0: i16, y0: i16, x1: i16, y1: i16);
    fn reset_clip(&mut self);
}

pub trait Buffer {
    fn clear(&mut self);
    fn present(&mut self);
    fn present_area(&mut self, x: u16, y: u16, width: u16, height: u16);
    fn buffer_ptr(&self) -> Option<*mut c_void>;
}

pub trait Driver: Input + Timing + Text + Draw + Buffer {}

impl<T> Driver for T where T: Input + Timing + Text + Draw + Buffer {}

pub trait RawHandle {
    fn as_raw_handle(&mut self) -> *mut c_void;
}
