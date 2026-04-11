#[path = "support/mod.rs"]
mod support;

use vision_ui::default_icon_pack;
use vision_ui_simulator_240x240::Simulator240x240;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let driver = Simulator240x240::new()?;

    support::run_demo(driver, |ui| {
        ui.set_title_font(Simulator240x240::title_font());
        ui.set_subtitle_font(Simulator240x240::subtitle_font());
        ui.set_body_font(Simulator240x240::body_font());
        ui.set_icon_pack(default_icon_pack());
        Ok(())
    })
}
