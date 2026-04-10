use std::error::Error;

use vision_ui as vui;

extern "C" fn on_toggle(value: bool) {
    let _ = value;
}

fn main() -> Result<(), Box<dyn Error>> {
    let root = vui::vision_ui_list_item_new_safe(8, false, "Root")?
        .ok_or("failed to create root item")?;

    let settings = vui::vision_ui_list_switch_item_new_safe("Enable feature", true, Some(on_toggle))?
        .ok_or("failed to create switch item")?;

    if !vui::vision_ui_list_push_item_safe(root, settings) {
        return Err("failed to push child item".into());
    }

    if !vui::vision_ui_root_item_set_safe(root) {
        return Err("failed to set root item".into());
    }

    vui::vision_ui_selector_bind_item_safe(root);
    vui::vision_ui_selector_go_next_item_safe();
    Ok(())
}
