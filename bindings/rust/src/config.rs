use core::time::Duration;

mod selected {
    include!(concat!(env!("OUT_DIR"), "/selected_config.rs"));
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ListFooterKind {
    Default,
    Slider,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DisplayConfig {
    pub width: u16,
    pub height: u16,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SystemConfig {
    pub allow_exit_by_user: bool,
    pub debug_overlay: bool,
    pub max_list_layer: u8,
    pub exit_animation_duration_ms: u32,
    pub enter_animation_duration_ms: u32,
}

impl SystemConfig {
    pub fn exit_animation_duration(self) -> Duration {
        Duration::from_millis(u64::from(self.exit_animation_duration_ms))
    }

    pub fn enter_animation_duration(self) -> Duration {
        Duration::from_millis(u64::from(self.enter_animation_duration_ms))
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct NotificationConfig {
    pub width: u16,
    pub height: u16,
    pub dismiss_duration_ms: u32,
}

impl NotificationConfig {
    pub fn dismiss_duration(self) -> Duration {
        Duration::from_millis(u64::from(self.dismiss_duration_ms))
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct AlertConfig {
    pub width: u16,
    pub height: u16,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ListConfig {
    pub entry_animation: bool,
    pub title_to_display_top_padding: u16,
    pub title_to_frame_padding: u16,
    pub frame_between_padding: u16,
    pub footer_right_to_scroll_bar_padding: u16,
    pub footer_to_left_padding: u16,
    pub header_to_text_padding: u16,
    pub header_to_left_display_padding: u16,
    pub selector_to_inner_widget_padding: u16,
    pub slider_value_scroll_speed_px_per_sec: u16,
    pub slider_value_scroll_pause_ms: u32,
    pub text_scroll_pause_ms: u32,
    pub text_scroll_speed_px_per_sec: u16,
    pub footer_max_height: u16,
    pub footer_max_width: u16,
    pub header_max_width: u16,
    pub slider_footer_width: u16,
    pub frame_height: u16,
    pub selector_height: u16,
    pub scroll_bar_width: u16,
    pub scroll_bar_animation_speed: u16,
}

impl ListConfig {
    pub fn slider_value_scroll_pause(self) -> Duration {
        Duration::from_millis(u64::from(self.slider_value_scroll_pause_ms))
    }

    pub fn text_scroll_pause(self) -> Duration {
        Duration::from_millis(u64::from(self.text_scroll_pause_ms))
    }

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

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct IconViewConfig {
    pub item_spacing: u16,
    pub title_bar_to_left_display_padding: u16,
    pub title_bar_to_title_padding: u16,
    pub title_to_right_display_min_padding: u16,
    pub icon_to_top_display_padding: u16,
    pub icon_to_title_area_padding: u16,
    pub title_area_to_description_padding: u16,
    pub description_area_height: u16,
    pub description_to_display_min_spacing: u16,
    pub scroll_speed: u16,
    pub icon_size: u16,
    pub title_area_height: u16,
    pub title_bar_width: u16,
}

pub const DISPLAY: DisplayConfig = DisplayConfig {
    width: selected::VISION_UI_SCREEN_WIDTH as u16,
    height: selected::VISION_UI_SCREEN_HEIGHT as u16,
};

pub const SYSTEM: SystemConfig = SystemConfig {
    allow_exit_by_user: selected::VISION_UI_ALLOW_EXIT_BY_USER != 0,
    debug_overlay: selected::DEBUG_OVERLAY,
    max_list_layer: selected::VISION_UI_MAX_LIST_LAYER as u8,
    exit_animation_duration_ms: selected::VISION_UI_EXIT_ANIMATION_DURATION_MS,
    enter_animation_duration_ms: selected::VISION_UI_ENTER_ANIMATION_DURATION_MS,
};

pub const NOTIFICATION: NotificationConfig = NotificationConfig {
    width: selected::VISION_UI_NOTIFICATION_WIDTH as u16,
    height: selected::VISION_UI_NOTIFICATION_HEIGHT as u16,
    dismiss_duration_ms: selected::VISION_UI_NOTIFICATION_DISMISS_DURATION_MS,
};

pub const ALERT: AlertConfig = AlertConfig {
    width: selected::VISION_UI_ALERT_WIDTH as u16,
    height: selected::VISION_UI_ALERT_HEIGHT as u16,
};

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

pub const SCREEN_WIDTH: u16 = DISPLAY.width;
pub const SCREEN_HEIGHT: u16 = DISPLAY.height;
pub const ICON_SIZE: u16 = ICON_VIEW.icon_size;
pub const ALLOW_EXIT_BY_USER: bool = SYSTEM.allow_exit_by_user;
pub const DEBUG_OVERLAY: bool = SYSTEM.debug_overlay;
