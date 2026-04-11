#[path = "support/mod.rs"]
mod support;

use core::convert::Infallible;
use core::ffi::{CStr, c_void};
use core::ptr;
use std::thread;
use std::time::{Duration, Instant};

use embedded_graphics::{
    Pixel,
    draw_target::DrawTarget,
    geometry::{OriginDimensions, Point, Size},
    image::{Image, ImageRaw},
    mono_font::{
        MonoFont, MonoTextStyle, MonoTextStyleBuilder,
        ascii::{FONT_6X10, FONT_8X13_BOLD, FONT_10X20},
    },
    pixelcolor::BinaryColor,
    prelude::*,
    primitives::{Circle, Line, PrimitiveStyle, Rectangle},
    text::{Baseline, Text},
};
use minifb::{Key, KeyRepeat, Scale, Window, WindowOptions};
use vision_ui::{
    Action, Font, SCREEN_HEIGHT, SCREEN_WIDTH, default_icon_pack,
    driver::{Buffer, Draw, Input, Text as DriverText, Timing},
};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let driver = EmbeddedGraphicsDriver::new(WindowDisplay::new(
        "Vision UI embedded-graphics driver",
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        4,
    )?);

    support::run_demo(driver, |ui| {
        ui.set_title_font(title_font());
        ui.set_subtitle_font(subtitle_font());
        ui.set_body_font(body_font());
        ui.set_icon_pack(default_icon_pack());
        Ok(())
    })
}

struct EmbeddedMonoFont {
    mono: &'static MonoFont<'static>,
}

impl EmbeddedMonoFont {
    const fn new(mono: &'static MonoFont<'static>) -> Self {
        Self { mono }
    }
}

static TITLE_FONT_FACE: EmbeddedMonoFont = EmbeddedMonoFont::new(&FONT_10X20);
static SUBTITLE_FONT_FACE: EmbeddedMonoFont = EmbeddedMonoFont::new(&FONT_8X13_BOLD);
static BODY_FONT_FACE: EmbeddedMonoFont = EmbeddedMonoFont::new(&FONT_6X10);

fn title_font() -> Font {
    font_handle(&TITLE_FONT_FACE)
}

fn subtitle_font() -> Font {
    font_handle(&SUBTITLE_FONT_FACE)
}

fn body_font() -> Font {
    font_handle(&BODY_FONT_FACE)
}

fn font_handle(font: &'static EmbeddedMonoFont) -> Font {
    Font {
        font: ptr::from_ref(font).cast::<c_void>(),
        top_compensation: 0,
        bottom_compensation: 0,
    }
}

struct WindowDisplay {
    width: usize,
    height: usize,
    pixels: Vec<BinaryColor>,
    frame: Vec<u32>,
    clip: Option<Rectangle>,
    draw_mode: DrawMode,
    window: Window,
}

impl WindowDisplay {
    fn new(title: &str, width: u16, height: u16, scale: usize) -> Result<Self, minifb::Error> {
        let width_usize = usize::from(width);
        let height_usize = usize::from(height);
        let mut window = Window::new(
            title,
            width_usize,
            height_usize,
            WindowOptions {
                resize: false,
                scale: match scale {
                    1 => Scale::X1,
                    2 => Scale::X2,
                    4 => Scale::X4,
                    8 => Scale::X8,
                    16 => Scale::X16,
                    32 => Scale::X32,
                    _ => Scale::FitScreen,
                },
                ..WindowOptions::default()
            },
        )?;
        window.set_target_fps(60);

        Ok(Self {
            width: width_usize,
            height: height_usize,
            pixels: vec![BinaryColor::Off; width_usize * height_usize],
            frame: vec![0x000000; width_usize * height_usize],
            clip: None,
            draw_mode: DrawMode::Set,
            window,
        })
    }

    fn set_clip(&mut self, clip: Option<Rectangle>) {
        self.clip = clip;
    }

    fn set_draw_mode(&mut self, draw_mode: DrawMode) {
        self.draw_mode = draw_mode;
    }

    fn clear(&mut self, color: BinaryColor) {
        self.pixels.fill(color);
    }

    fn present(&mut self) {
        for (src, dst) in self.pixels.iter().zip(self.frame.iter_mut()) {
            *dst = match src {
                BinaryColor::Off => 0x101010,
                BinaryColor::On => 0xF2F2F2,
            };
        }

        let _ = self
            .window
            .update_with_buffer(&self.frame, self.width, self.height);
    }

    fn poll_action(&self) -> Action {
        if !self.window.is_open()
            || self.window.is_key_pressed(Key::Escape, KeyRepeat::No)
            || self.window.is_key_pressed(Key::Q, KeyRepeat::No)
        {
            Action::Exit
        } else if self.window.is_key_pressed(Key::Up, KeyRepeat::No)
            || self.window.is_key_pressed(Key::Left, KeyRepeat::No)
        {
            Action::Previous
        } else if self.window.is_key_pressed(Key::Down, KeyRepeat::No)
            || self.window.is_key_pressed(Key::Right, KeyRepeat::No)
        {
            Action::Next
        } else if self.window.is_key_pressed(Key::Enter, KeyRepeat::No)
            || self.window.is_key_pressed(Key::Space, KeyRepeat::No)
        {
            Action::Enter
        } else {
            Action::None
        }
    }

    fn point_visible(&self, point: Point) -> bool {
        point.x >= 0
            && point.y >= 0
            && point.x < self.width as i32
            && point.y < self.height as i32
            && self.clip.is_none_or(|clip| clip.contains(point))
    }

    fn pixel_index(&self, point: Point) -> usize {
        point.y as usize * self.width + point.x as usize
    }
}

impl DrawTarget for WindowDisplay {
    type Color = BinaryColor;
    type Error = Infallible;

    fn draw_iter<I>(&mut self, pixels: I) -> Result<(), Self::Error>
    where
        I: IntoIterator<Item = Pixel<Self::Color>>,
    {
        for Pixel(point, color) in pixels {
            if !self.point_visible(point) {
                continue;
            }

            let index = self.pixel_index(point);
            self.pixels[index] = match self.draw_mode {
                DrawMode::Clear | DrawMode::Set => color,
                DrawMode::Xor => match self.pixels[index] {
                    BinaryColor::Off => BinaryColor::On,
                    BinaryColor::On => BinaryColor::Off,
                },
            };
        }

        Ok(())
    }
}

impl OriginDimensions for WindowDisplay {
    fn size(&self) -> Size {
        Size::new(self.width as u32, self.height as u32)
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum DrawMode {
    Clear,
    Set,
    Xor,
}

struct EmbeddedGraphicsDriver {
    display: WindowDisplay,
    current_font: Font,
    draw_mode: DrawMode,
    font_mode: u8,
    font_direction: u8,
    clip: Option<Rectangle>,
    started_at: Instant,
}

impl EmbeddedGraphicsDriver {
    fn new(display: WindowDisplay) -> Self {
        Self {
            display,
            current_font: body_font(),
            draw_mode: DrawMode::Set,
            font_mode: 0,
            font_direction: 0,
            clip: None,
            started_at: Instant::now(),
        }
    }

    fn sync_state(&mut self) {
        self.display.set_clip(self.clip);
        self.display.set_draw_mode(self.draw_mode);
    }

    fn draw_color(&self) -> BinaryColor {
        match self.draw_mode {
            DrawMode::Clear => BinaryColor::Off,
            DrawMode::Set | DrawMode::Xor => BinaryColor::On,
        }
    }

    fn active_font(&self) -> &'static EmbeddedMonoFont {
        if self.current_font.font.is_null() {
            &BODY_FONT_FACE
        } else {
            unsafe { &*self.current_font.font.cast::<EmbeddedMonoFont>() }
        }
    }

    fn text_style(&self) -> MonoTextStyle<'static, BinaryColor> {
        let builder = MonoTextStyleBuilder::new()
            .font(self.active_font().mono)
            .text_color(self.draw_color());

        if self.font_mode == 0 {
            builder.build()
        } else {
            builder
                .background_color(background_color(self.draw_color()))
                .build()
        }
    }

    fn adjusted_baseline(&self, y: u16) -> i32 {
        i32::from(y) - i32::from(self.current_font.bottom_compensation)
    }
}

impl Input for EmbeddedGraphicsDriver {
    fn action(&mut self) -> Action {
        self.display.poll_action()
    }
}

impl Timing for EmbeddedGraphicsDriver {
    fn ticks(&mut self) -> Duration {
        self.started_at.elapsed()
    }

    fn delay(&mut self, duration: Duration) {
        thread::sleep(duration);
    }
}

impl DriverText for EmbeddedGraphicsDriver {
    fn set_font(&mut self, font: Font) {
        self.current_font = font;
    }

    fn font(&self) -> Font {
        self.current_font
    }

    fn draw_text(&mut self, x: u16, y: u16, text: &CStr) {
        self.sync_state();

        let value = text.to_str().unwrap_or_default();
        let position = Point::new(i32::from(x), self.adjusted_baseline(y));
        let _ = Text::with_baseline(value, position, self.text_style(), Baseline::Alphabetic)
            .draw(&mut self.display);
    }

    fn draw_utf8(&mut self, x: u16, y: u16, text: &CStr) {
        self.draw_text(x, y, text);
    }

    fn text_width(&self, text: &CStr) -> u16 {
        let value = text.to_str().unwrap_or_default();
        let bounds = Text::with_baseline(
            value,
            Point::zero(),
            self.text_style(),
            Baseline::Alphabetic,
        )
        .bounding_box();
        bounds.size.width.min(u32::from(u16::MAX)) as u16
    }

    fn utf8_width(&self, text: &CStr) -> u16 {
        self.text_width(text)
    }

    fn text_height(&self) -> u16 {
        let font = self.active_font().mono;
        let adjusted = (font.character_size.height.min(i32::MAX as u32) as i32)
            + i32::from(self.current_font.top_compensation);
        adjusted.clamp(0, i32::from(u16::MAX)) as u16
    }

    fn set_font_mode(&mut self, mode: u8) {
        self.font_mode = mode;
    }

    fn set_font_direction(&mut self, direction: u8) {
        self.font_direction = direction & 0x03;
    }
}

impl Draw for EmbeddedGraphicsDriver {
    fn pixel(&mut self, x: u16, y: u16) {
        self.sync_state();
        let _ = Pixel(Point::new(i32::from(x), i32::from(y)), self.draw_color())
            .draw(&mut self.display);
    }

    fn circle(&mut self, x: u16, y: u16, radius: u16) {
        self.sync_state();

        let diameter = u32::from(radius) * 2;
        let top_left = Point::new(
            i32::from(x) - i32::from(radius),
            i32::from(y) - i32::from(radius),
        );
        let circle = Circle::new(top_left, diameter)
            .into_styled(PrimitiveStyle::with_stroke(self.draw_color(), 1));
        let _ = circle.draw(&mut self.display);
    }

    fn disc(&mut self, x: u16, y: u16, radius: u16) {
        self.sync_state();

        let diameter = u32::from(radius) * 2;
        let top_left = Point::new(
            i32::from(x) - i32::from(radius),
            i32::from(y) - i32::from(radius),
        );
        let circle = Circle::new(top_left, diameter)
            .into_styled(PrimitiveStyle::with_fill(self.draw_color()));
        let _ = circle.draw(&mut self.display);
    }

    fn fill_rounded_rect(&mut self, x: u16, y: u16, width: u16, height: u16, _radius: u16) {
        self.fill_rect(x, y, width, height);
    }

    fn fill_rect(&mut self, x: u16, y: u16, width: u16, height: u16) {
        self.sync_state();

        let rect = Rectangle::new(
            Point::new(i32::from(x), i32::from(y)),
            Size::new(u32::from(width), u32::from(height)),
        )
        .into_styled(PrimitiveStyle::with_fill(self.draw_color()));
        let _ = rect.draw(&mut self.display);
    }

    fn stroke_rect(&mut self, x: u16, y: u16, width: u16, height: u16) {
        self.sync_state();

        let rect = Rectangle::new(
            Point::new(i32::from(x), i32::from(y)),
            Size::new(u32::from(width), u32::from(height)),
        )
        .into_styled(PrimitiveStyle::with_stroke(self.draw_color(), 1));
        let _ = rect.draw(&mut self.display);
    }

    fn stroke_rounded_rect(&mut self, x: u16, y: u16, width: u16, height: u16, _radius: u16) {
        self.stroke_rect(x, y, width, height);
    }

    fn hline(&mut self, x: u16, y: u16, length: u16) {
        self.line(x, y, x.saturating_add(length.saturating_sub(1)), y);
    }

    fn vline(&mut self, x: u16, y: u16, height: u16) {
        self.line(x, y, x, y.saturating_add(height.saturating_sub(1)));
    }

    fn line(&mut self, x1: u16, y1: u16, x2: u16, y2: u16) {
        self.sync_state();

        let line = Line::new(
            Point::new(i32::from(x1), i32::from(y1)),
            Point::new(i32::from(x2), i32::from(y2)),
        )
        .into_styled(PrimitiveStyle::with_stroke(self.draw_color(), 1));
        let _ = line.draw(&mut self.display);
    }

    fn dotted_hline(&mut self, x: u16, y: u16, length: u16) {
        for offset in (0..length).step_by(2) {
            self.pixel(x.saturating_add(offset), y);
        }
    }

    fn dotted_vline(&mut self, x: u16, y: u16, height: u16) {
        for offset in (0..height).step_by(2) {
            self.pixel(x, y.saturating_add(offset));
        }
    }

    fn bitmap(&mut self, x: u16, y: u16, width: u16, _height: u16, bitmap: &[u8]) {
        self.sync_state();

        let raw = ImageRaw::<BinaryColor>::new(bitmap, u32::from(width));
        let image = Image::new(&raw, Point::new(i32::from(x), i32::from(y)));
        let _ = image.draw(&mut self.display);
    }

    fn set_color(&mut self, color: u8) {
        self.draw_mode = match color {
            0 => DrawMode::Clear,
            1 => DrawMode::Set,
            2 => DrawMode::Xor,
            _ => DrawMode::Set,
        };
    }

    fn clip_rect(&mut self, x0: i16, y0: i16, x1: i16, y1: i16) {
        let width = i32::from(x1).saturating_sub(i32::from(x0)).max(0) as u32;
        let height = i32::from(y1).saturating_sub(i32::from(y0)).max(0) as u32;

        self.clip = Some(Rectangle::new(
            Point::new(i32::from(x0), i32::from(y0)),
            Size::new(width, height),
        ));
    }

    fn reset_clip(&mut self) {
        self.clip = None;
    }
}

impl Buffer for EmbeddedGraphicsDriver {
    fn clear(&mut self) {
        self.display.clear(BinaryColor::Off);
    }

    fn present(&mut self) {
        self.display.present();
    }

    fn present_area(&mut self, _x: u16, _y: u16, _width: u16, _height: u16) {
        self.present();
    }

    fn buffer_ptr(&self) -> Option<*mut c_void> {
        None
    }
}

fn background_color(color: BinaryColor) -> BinaryColor {
    match color {
        BinaryColor::Off => BinaryColor::On,
        BinaryColor::On => BinaryColor::Off,
    }
}
