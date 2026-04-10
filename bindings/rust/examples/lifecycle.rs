use vision_ui as vui;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    vui::vision_ui_render_init_safe();
    vui::vision_ui_core_init_safe();

    // Optional startup logo.
    let logo = [0_u8; 128];
    vui::vision_ui_start_logo_set_safe(&logo)?;

    // Typical app loop step.
    vui::vision_ui_step_render_safe();

    if !vui::vision_ui_is_exited_safe() {
        vui::vision_ui_notification_push_safe("Vision UI started", 1200)?;
    }

    Ok(())
}
