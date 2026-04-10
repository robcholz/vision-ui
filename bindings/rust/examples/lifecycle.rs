use vision_ui as vui;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    vui::vision_ui_render_init();
    vui::vision_ui_core_init();

    // Optional startup logo.
    let logo = [0_u8; 128];
    vui::vision_ui_start_logo_set(&logo)?;

    // Typical app loop step.
    vui::vision_ui_step_render();

    if !vui::vision_ui_is_exited() {
        vui::vision_ui_notification_push("Vision UI started", 1200)?;
    }

    Ok(())
}
