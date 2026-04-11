use core::time::Duration;

mod selected {
    include!(concat!(env!("OUT_DIR"), "/selected_config.rs"));
}

/// Footer shape used when deriving list text width.
///
/// List rows reserve a different amount of space on the right depending on whether
/// they show a normal footer or a slider value footer.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ListFooterKind {
    /// A normal list row with the standard footer area.
    Default,
    /// A slider row whose value footer is narrower than the standard footer area.
    Slider,
}

/// Compiled display dimensions.
///
/// These values define the pixel size that the renderer and layout code target.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DisplayConfig {
    /// Display width in pixels.
    pub width: u16,
    /// Display height in pixels.
    pub height: u16,
}

/// Compiled system-level behavior settings.
///
/// These values control top-level runtime behavior such as exiting and transition timing.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SystemConfig {
    /// Whether the user is allowed to exit the UI from the top level.
    pub allow_exit_by_user: bool,

    /// Reflects whether the native C build enabled the `DEBUG` overlay path.
    ///
    /// When enabled, Vision UI draws outlines for key layout regions.
    pub debug_overlay: bool,

    /// Maximum supported nesting depth for list items.
    pub max_list_layer: u8,

    /// Exit animation duration in milliseconds.
    pub exit_animation_duration_ms: u32,

    /// Enter animation duration in milliseconds.
    pub enter_animation_duration_ms: u32,
}

impl SystemConfig {
    /// Returns the configured exit animation duration.
    pub fn exit_animation_duration(self) -> Duration {
        Duration::from_millis(u64::from(self.exit_animation_duration_ms))
    }

    /// Returns the configured enter animation duration.
    pub fn enter_animation_duration(self) -> Duration {
        Duration::from_millis(u64::from(self.enter_animation_duration_ms))
    }
}

/// Compiled notification layout and timing settings.
///
/// These values define the notification overlay size and how long it stays visible.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct NotificationConfig {
    /// Notification overlay width in pixels.
    pub width: u16,

    /// Notification overlay height in pixels.
    pub height: u16,

    /// Notification dismiss duration in milliseconds.
    pub dismiss_duration_ms: u32,
}

impl NotificationConfig {
    /// Returns the configured notification dismiss duration.
    pub fn dismiss_duration(self) -> Duration {
        Duration::from_millis(u64::from(self.dismiss_duration_ms))
    }
}

/// Compiled alert layout settings.
///
/// These values define the size of the centered alert overlay.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct AlertConfig {
    /// Alert overlay width in pixels.
    pub width: u16,

    /// Alert overlay height in pixels.
    pub height: u16,
}

/// Compiled list-view layout, animation, and scrolling settings.
///
/// These values define spacing, fixed dimensions, and scroll timing for the list renderer.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ListConfig {
    /// Whether list rows animate into place when entering a new list.
    pub entry_animation: bool,

    /// Vertical padding between the top display edge and the list title.
    pub title_to_display_top_padding: u16,

    /// Vertical padding between the title and the first item frame.
    pub title_to_frame_padding: u16,

    /// Vertical spacing between adjacent item frames.
    pub frame_between_padding: u16,

    /// Horizontal padding between the footer area and the scroll bar.
    pub footer_right_to_scroll_bar_padding: u16,

    /// Minimum spacing between the footer center and the right edge of the text area.
    pub footer_to_left_padding: u16,

    /// Horizontal padding between the header area and the text area.
    pub header_to_text_padding: u16,

    /// Horizontal padding between the left display edge and the header area.
    pub header_to_left_display_padding: u16,

    /// Padding between the selector outline and the inner widget content.
    pub selector_to_inner_widget_padding: u16,

    /// Scroll speed for slider values, in pixels per second.
    pub slider_value_scroll_speed_px_per_sec: u16,

    /// Pause duration applied when a scrolling slider value reaches either end.
    pub slider_value_scroll_pause_ms: u32,

    /// Pause duration applied when scrolling row text reaches either end.
    pub text_scroll_pause_ms: u32,

    /// Scroll speed for row text, in pixels per second.
    pub text_scroll_speed_px_per_sec: u16,

    /// Maximum footer height in pixels.
    pub footer_max_height: u16,

    /// Maximum width reserved for a standard footer.
    pub footer_max_width: u16,

    /// Maximum width reserved for the header area.
    pub header_max_width: u16,

    /// Width reserved for a slider value footer.
    pub slider_footer_width: u16,

    /// Fixed row frame height.
    pub frame_height: u16,

    /// Fixed selector height.
    pub selector_height: u16,

    /// Scroll bar width in pixels.
    pub scroll_bar_width: u16,

    /// Scroll bar animation speed used by the renderer.
    pub scroll_bar_animation_speed: u16,
}

impl ListConfig {
    /// Returns the configured slider-value scroll pause.
    pub fn slider_value_scroll_pause(self) -> Duration {
        Duration::from_millis(u64::from(self.slider_value_scroll_pause_ms))
    }

    /// Returns the configured list-text scroll pause.
    pub fn text_scroll_pause(self) -> Duration {
        Duration::from_millis(u64::from(self.text_scroll_pause_ms))
    }

    /// Computes the maximum width available for list row text.
    ///
    /// Parameters:
    /// - `display`: compiled display dimensions.
    /// - `footer_kind`: whether the row uses the default footer width or slider footer width.
    ///
    /// Returns:
    /// - the maximum text width in pixels after subtracting footer, header, and padding areas.
    ///
    /// Behavior:
    /// - This follows the same layout math used by the native renderer.
    pub fn text_max_width(self, display: DisplayConfig, footer_kind: ListFooterKind) -> u16 {
        let footer_width = match footer_kind {
            ListFooterKind::Default => self.footer_max_width,
            ListFooterKind::Slider => self.slider_footer_width,
        };

        display
            .width
            .saturating_sub(self.footer_right_to_scroll_bar_padding)
            .saturating_sub(footer_width)
            .saturating_sub(self.footer_to_left_padding)
            .saturating_sub(self.header_to_text_padding)
            .saturating_sub(self.header_max_width)
            .saturating_sub(self.header_to_left_display_padding)
    }
}

/// Compiled icon-view layout and scrolling settings.
///
/// These values define spacing, reserved regions, and scroll behavior for the icon view.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct IconViewConfig {
    /// Horizontal spacing between icon items.
    pub item_spacing: u16,

    /// Horizontal padding from the title bar to the left display edge.
    pub title_bar_to_left_display_padding: u16,

    /// Horizontal padding from the title bar to the title text.
    pub title_bar_to_title_padding: u16,

    /// Minimum horizontal padding from the title text to the right display edge.
    pub title_to_right_display_min_padding: u16,

    /// Vertical padding from the top display edge to the icon.
    pub icon_to_top_display_padding: u16,

    /// Vertical padding between the icon and the title area.
    pub icon_to_title_area_padding: u16,

    /// Vertical padding between the title area and the description area.
    pub title_area_to_description_padding: u16,

    /// Height reserved for the description area.
    pub description_area_height: u16,

    /// Minimum horizontal padding from the display edges to the description text.
    pub description_to_display_min_spacing: u16,

    /// Scroll speed used for icon and title movement.
    pub scroll_speed: u16,

    /// Icon size in pixels.
    pub icon_size: u16,

    /// Height reserved for the title area.
    pub title_area_height: u16,

    /// Width reserved for the title bar.
    pub title_bar_width: u16,
}

/// Display dimensions selected at build time.
pub const DISPLAY: DisplayConfig = DisplayConfig {
    width: selected::VISION_UI_SCREEN_WIDTH as u16,
    height: selected::VISION_UI_SCREEN_HEIGHT as u16,
};

/// Top-level system behavior selected at build time.
pub const SYSTEM: SystemConfig = SystemConfig {
    allow_exit_by_user: selected::VISION_UI_ALLOW_EXIT_BY_USER != 0,
    debug_overlay: selected::DEBUG_OVERLAY,
    max_list_layer: selected::VISION_UI_MAX_LIST_LAYER as u8,
    exit_animation_duration_ms: selected::VISION_UI_EXIT_ANIMATION_DURATION_MS,
    enter_animation_duration_ms: selected::VISION_UI_ENTER_ANIMATION_DURATION_MS,
};

/// Notification layout and timing selected at build time.
pub const NOTIFICATION: NotificationConfig = NotificationConfig {
    width: selected::VISION_UI_NOTIFICATION_WIDTH as u16,
    height: selected::VISION_UI_NOTIFICATION_HEIGHT as u16,
    dismiss_duration_ms: selected::VISION_UI_NOTIFICATION_DISMISS_DURATION_MS,
};

/// Alert layout selected at build time.
pub const ALERT: AlertConfig = AlertConfig {
    width: selected::VISION_UI_ALERT_WIDTH as u16,
    height: selected::VISION_UI_ALERT_HEIGHT as u16,
};

/// List-view layout, animation, and scroll settings selected at build time.
pub const LIST_VIEW: ListConfig = ListConfig {
    entry_animation: selected::VISION_UI_LIST_ENTRY_ANIMATION != 0,
    title_to_display_top_padding: selected::VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING as u16,
    title_to_frame_padding: selected::VISION_UI_LIST_TITLE_TO_FRAME_PADDING as u16,
    frame_between_padding: selected::VISION_UI_LIST_FRAME_BETWEEN_PADDING as u16,
    footer_right_to_scroll_bar_padding: selected::VISION_UI_LIST_FOOTER_RIGHT_TO_SCROLL_BAR_PADDING
        as u16,
    footer_to_left_padding: selected::VISION_UI_LIST_FOOTER_TO_LEFT_PADDING as u16,
    header_to_text_padding: selected::VISION_UI_LIST_HEADER_TO_TEXT_PADDING as u16,
    header_to_left_display_padding: selected::VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING as u16,
    selector_to_inner_widget_padding: selected::VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING
        as u16,
    slider_value_scroll_speed_px_per_sec: selected::VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S
        as u16,
    slider_value_scroll_pause_ms: selected::VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS,
    text_scroll_pause_ms: selected::VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS,
    text_scroll_speed_px_per_sec: selected::VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S as u16,
    footer_max_height: selected::VISION_UI_LIST_FOOTER_MAX_HEIGHT as u16,
    footer_max_width: selected::VISION_UI_LIST_FOOTER_MAX_WIDTH as u16,
    header_max_width: selected::VISION_UI_LIST_HEADER_MAX_WIDTH as u16,
    slider_footer_width: selected::VISION_UI_LIST_SLIDER_FOOTER_WIDTH as u16,
    frame_height: selected::VISION_UI_LIST_FRAME_FIXED_HEIGHT as u16,
    selector_height: selected::VISION_UI_LIST_SELECTOR_FIXED_HEIGHT as u16,
    scroll_bar_width: selected::VISION_UI_LIST_SCROLL_BAR_WIDTH as u16,
    scroll_bar_animation_speed: selected::VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED as u16,
};

/// Icon-view layout and scroll settings selected at build time.
pub const ICON_VIEW: IconViewConfig = IconViewConfig {
    item_spacing: selected::VISION_UI_ICON_VIEW_ITEM_SPACING as u16,
    title_bar_to_left_display_padding:
        selected::VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING as u16,
    title_bar_to_title_padding: selected::VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING as u16,
    title_to_right_display_min_padding:
        selected::VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING as u16,
    icon_to_top_display_padding: selected::VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING as u16,
    icon_to_title_area_padding: selected::VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING as u16,
    title_area_to_description_padding:
        selected::VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING as u16,
    description_area_height: selected::VISION_UI_ICON_VIEW_DESCRIPTION_AREA_HEIGHT as u16,
    description_to_display_min_spacing:
        selected::VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING as u16,
    scroll_speed: selected::VISION_UI_ICON_VIEW_SCROLL_SPEED as u16,
    icon_size: selected::VISION_UI_ICON_VIEW_ICON_SIZE as u16,
    title_area_height: selected::VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT as u16,
    title_bar_width: selected::VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH as u16,
};

/// Display width in pixels.
pub const SCREEN_WIDTH: u16 = DISPLAY.width;

/// Display height in pixels.
pub const SCREEN_HEIGHT: u16 = DISPLAY.height;

/// Icon size in pixels used by the icon view.
pub const ICON_SIZE: u16 = ICON_VIEW.icon_size;

/// Whether the user can exit from the top level.
pub const ALLOW_EXIT_BY_USER: bool = SYSTEM.allow_exit_by_user;

/// Whether debug outlines are enabled in the native renderer.
pub const DEBUG_OVERLAY: bool = SYSTEM.debug_overlay;
