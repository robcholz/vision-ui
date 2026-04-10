use std::error::Error;

use vision_ui as vui;

extern "C" fn on_toggle(value: bool) {
    let _ = value;
}

fn main() -> Result<(), Box<dyn Error>> {
    let root = vui::vision_ui_list_item_new(8, false, "Root")?.ok_or("failed to create root item")?;

    let settings = vui::vision_ui_list_switch_item_new("Enable feature", true, Some(on_toggle))?
        .ok_or("failed to create switch item")?;

    if !vui::vision_ui_list_push_item(root, settings) {
        return Err("failed to push child item".into());
    }

    if !vui::vision_ui_root_item_set(root) {
        return Err("failed to set root item".into());
    }

    Ok(())
}
