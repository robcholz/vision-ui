//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_CONFIG_H
#define VISION_UI_VISION_UI_CONFIG_H

// screen config
#ifdef CONFIG_VISION_UI_SCREEN_HEIGHT
#define VISION_UI_SCREEN_HEIGHT CONFIG_VISION_UI_SCREEN_HEIGHT
#else
#define VISION_UI_SCREEN_HEIGHT 240 // the width of the screen
#endif

#ifdef CONFIG_VISION_UI_SCREEN_WIDTH
#define VISION_UI_SCREEN_WIDTH CONFIG_VISION_UI_SCREEN_WIDTH
#else
#define VISION_UI_SCREEN_WIDTH 240 // the height of the screen
#endif

// system
#ifdef CONFIG_VISION_UI_ALLOW_EXIT_BY_USER
#define VISION_UI_ALLOW_EXIT_BY_USER CONFIG_VISION_UI_ALLOW_EXIT_BY_USER
#else
#define VISION_UI_ALLOW_EXIT_BY_USER 0 // allow user to exit vision ui at the top level
#endif

#ifdef CONFIG_VISION_UI_MAX_LIST_LAYER
#define VISION_UI_MAX_LIST_LAYER CONFIG_VISION_UI_MAX_LIST_LAYER
#else
#define VISION_UI_MAX_LIST_LAYER 10 // the max layer of the ui
#endif

#ifdef CONFIG_VISION_UI_EXIT_ANIMATION_DURATION_MS
#define VISION_UI_EXIT_ANIMATION_DURATION_MS CONFIG_VISION_UI_EXIT_ANIMATION_DURATION_MS
#else
#define VISION_UI_EXIT_ANIMATION_DURATION_MS 180 // the time taken for the exit animation
#endif

#ifdef CONFIG_VISION_UI_ENTER_ANIMATION_DURATION_MS
#define VISION_UI_ENTER_ANIMATION_DURATION_MS CONFIG_VISION_UI_ENTER_ANIMATION_DURATION_MS
#else
#define VISION_UI_ENTER_ANIMATION_DURATION_MS 200 // the time taken for the enter animation
#endif

// notification
#ifdef CONFIG_VISION_UI_NOTIFICATION_HEIGHT
#define VISION_UI_NOTIFICATION_HEIGHT CONFIG_VISION_UI_NOTIFICATION_HEIGHT
#else
#define VISION_UI_NOTIFICATION_HEIGHT 15 // the height of the notification
#endif

#ifdef CONFIG_VISION_UI_NOTIFICATION_WIDTH
#define VISION_UI_NOTIFICATION_WIDTH CONFIG_VISION_UI_NOTIFICATION_WIDTH
#else
#define VISION_UI_NOTIFICATION_WIDTH 20 // the width of the notification
#endif

#ifdef CONFIG_VISION_UI_NOTIFICATION_DISMISS_DURATION_MS
#define VISION_UI_NOTIFICATION_DISMISS_DURATION_MS CONFIG_VISION_UI_NOTIFICATION_DISMISS_DURATION_MS
#else
#define VISION_UI_NOTIFICATION_DISMISS_DURATION_MS 1500 // the dismiss time of the notification
#endif

// alert
#ifdef CONFIG_VISION_UI_ALERT_HEIGHT
#define VISION_UI_ALERT_HEIGHT CONFIG_VISION_UI_ALERT_HEIGHT
#else
#define VISION_UI_ALERT_HEIGHT 20 // the height of the alert
#endif

#ifdef CONFIG_VISION_UI_ALERT_WIDTH
#define VISION_UI_ALERT_WIDTH CONFIG_VISION_UI_ALERT_WIDTH
#else
#define VISION_UI_ALERT_WIDTH 20 // the width of the alert
#endif

// list view
#ifdef CONFIG_VISION_UI_LIST_ENTRY_ANIMATION
#define VISION_UI_LIST_ENTRY_ANIMATION CONFIG_VISION_UI_LIST_ENTRY_ANIMATION
#else
#define VISION_UI_LIST_ENTRY_ANIMATION 0 // control whether items animate into place when a new list opens
#endif

#ifdef CONFIG_VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING
#define VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING CONFIG_VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING
#else
#define VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING 0 // the vertical padding between the top display edge to the title
#endif

#ifdef CONFIG_VISION_UI_LIST_TITLE_TO_FRAME_PADDING
#define VISION_UI_LIST_TITLE_TO_FRAME_PADDING CONFIG_VISION_UI_LIST_TITLE_TO_FRAME_PADDING
#else
#define VISION_UI_LIST_TITLE_TO_FRAME_PADDING 4 // the vertical padding between the title to the item frame
#endif

#ifdef CONFIG_VISION_UI_LIST_FRAME_BETWEEN_PADDING
#define VISION_UI_LIST_FRAME_BETWEEN_PADDING CONFIG_VISION_UI_LIST_FRAME_BETWEEN_PADDING
#else
#define VISION_UI_LIST_FRAME_BETWEEN_PADDING 2 // the vertical padding between each item frame in the list view
#endif

#ifdef CONFIG_VISION_UI_LIST_FOOTER_CENTER_TO_SCROLL_BAR_PADDING
#define VISION_UI_LIST_FOOTER_CENTER_TO_SCROLL_BAR_PADDING CONFIG_VISION_UI_LIST_FOOTER_CENTER_TO_SCROLL_BAR_PADDING
#else
#define VISION_UI_LIST_FOOTER_CENTER_TO_SCROLL_BAR_PADDING                                                             \
    10 // the horizontal padding between the scroll bar and the center of the footer
#endif

#ifdef CONFIG_VISION_UI_LIST_FOOTER_TO_LEFT_PADDING
#define VISION_UI_LIST_FOOTER_TO_LEFT_PADDING CONFIG_VISION_UI_LIST_FOOTER_TO_LEFT_PADDING
#else
#define VISION_UI_LIST_FOOTER_TO_LEFT_PADDING                                                                          \
    10 // the min horizontal padding between the footer center and the right of the text area
#endif

#ifdef CONFIG_VISION_UI_LIST_HEADER_TO_TEXT_PADDING
#define VISION_UI_LIST_HEADER_TO_TEXT_PADDING CONFIG_VISION_UI_LIST_HEADER_TO_TEXT_PADDING
#else
#define VISION_UI_LIST_HEADER_TO_TEXT_PADDING                                                                          \
    2 // the horizontal padding between the right of the header to the left of the text
#endif

#ifdef CONFIG_VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING
#define VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING CONFIG_VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING
#else
#define VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING                                                                  \
    4 // the horizontal padding between the left display edge to the header
#endif

#ifdef CONFIG_VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING
#define VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING CONFIG_VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING
#else
#define VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING                                                                \
    3 // the padding between the edge of the selector to the inner widget
#endif

#ifdef CONFIG_VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S
#define VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S CONFIG_VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S
#else
#define VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S                                                                  \
    5 // the speed of scrolling of the slider value (if it slides!), in pixel/second
#endif

#ifdef CONFIG_VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS
#define VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS CONFIG_VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS
#else
#define VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS                                                                    \
    1500 // the time to pause when the scrolling text reaches both end of the scrolling area
#endif

#ifdef CONFIG_VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS
#define VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS CONFIG_VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS
#else
#define VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS                                                                            \
    1000 // the time to pause when the scrolling text reaches both end of the scrolling area
#endif

#ifdef CONFIG_VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S
#define VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S CONFIG_VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S
#else
#define VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S 15 // the scrolling speed of text, in pixel per second
#endif

#ifdef CONFIG_VISION_UI_LIST_FOOTER_MAX_HEIGHT
#define VISION_UI_LIST_FOOTER_MAX_HEIGHT CONFIG_VISION_UI_LIST_FOOTER_MAX_HEIGHT
#else
#define VISION_UI_LIST_FOOTER_MAX_HEIGHT                                                                               \
    11 // the max height of the footer, this should be smaller than the frame height
#endif

#ifdef CONFIG_VISION_UI_LIST_FOOTER_MAX_WIDTH
#define VISION_UI_LIST_FOOTER_MAX_WIDTH CONFIG_VISION_UI_LIST_FOOTER_MAX_WIDTH
#else
#define VISION_UI_LIST_FOOTER_MAX_WIDTH 19 // the max width of the footer
#endif

#ifdef CONFIG_VISION_UI_LIST_HEADER_MAX_HEIGHT
#define VISION_UI_LIST_HEADER_MAX_HEIGHT CONFIG_VISION_UI_LIST_HEADER_MAX_HEIGHT
#else
#define VISION_UI_LIST_HEADER_MAX_HEIGHT 7 // the max height of the header, this should be smaller than the frame height
#endif

#ifdef CONFIG_VISION_UI_LIST_HEADER_MAX_WIDTH
#define VISION_UI_LIST_HEADER_MAX_WIDTH CONFIG_VISION_UI_LIST_HEADER_MAX_WIDTH
#else
#define VISION_UI_LIST_HEADER_MAX_WIDTH 7 // the max width of the footer
#endif

#ifdef CONFIG_VISION_UI_LIST_FRAME_FIXED_HEIGHT
#define VISION_UI_LIST_FRAME_FIXED_HEIGHT CONFIG_VISION_UI_LIST_FRAME_FIXED_HEIGHT
#else
#define VISION_UI_LIST_FRAME_FIXED_HEIGHT 15 // the fixed frame height
#endif

#ifdef CONFIG_VISION_UI_LIST_SELECTOR_FIXED_HEIGHT
#define VISION_UI_LIST_SELECTOR_FIXED_HEIGHT CONFIG_VISION_UI_LIST_SELECTOR_FIXED_HEIGHT
#else
#define VISION_UI_LIST_SELECTOR_FIXED_HEIGHT                                                                           \
    15 // the fixed height of the selector, this should typically equal to the frame height
#endif

// icon view
#ifdef CONFIG_VISION_UI_ICON_VIEW_ITEM_SPACING
#define VISION_UI_ICON_VIEW_ITEM_SPACING CONFIG_VISION_UI_ICON_VIEW_ITEM_SPACING
#else
#define VISION_UI_ICON_VIEW_ITEM_SPACING 15 // the horizontal padding between the icon
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING
#define VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING                                                          \
    CONFIG_VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING
#else
#define VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING                                                          \
    0 // the horizontal padding from the title bar to the left edge of the display
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING
#define VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING CONFIG_VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING
#else
#define VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING                                                                 \
    10 // the horizontal padding from the right of the title bar to the right title text
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING
#define VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING                                                         \
    CONFIG_VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING
#else
#define VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING                                                         \
    15 // the min horizontal padding from the title text to the display right edge
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING
#define VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING CONFIG_VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING
#else
#define VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING 5 // the vertical padding from the top display edge to the icon
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING
#define VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING CONFIG_VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING
#else
#define VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING 10 // the vertical padding between the icon and the title area
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING
#define VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING                                                          \
    CONFIG_VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING
#else
#define VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING                                                          \
    20 // the vertical padding between the title area and the description area
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING
#define VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING                                                         \
    CONFIG_VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING
#else
#define VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING                                                         \
    3 // the min horizontal padding from each side of the display to description
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_SCROLL_SPEED
#define VISION_UI_ICON_VIEW_SCROLL_SPEED CONFIG_VISION_UI_ICON_VIEW_SCROLL_SPEED
#else
#define VISION_UI_ICON_VIEW_SCROLL_SPEED 85 // the scrolling speed of the icon and the icon view title
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_ICON_SIZE
#define VISION_UI_ICON_VIEW_ICON_SIZE CONFIG_VISION_UI_ICON_VIEW_ICON_SIZE
#else
#define VISION_UI_ICON_VIEW_ICON_SIZE 100 // the length of the side of the icon
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT
#define VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT CONFIG_VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT
#else
#define VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT 70 // the height of the title area
#endif

#ifdef CONFIG_VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH
#define VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH CONFIG_VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH
#else
#define VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH 9 // the width of the title bar
#endif

// ********** DO NOT EDIT LINES BELOW **********

// fixed properties
#define VISION_UI_LIST_SCROLL_BAR_WIDTH 3
#define VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED 92

// derived properties
#define VISION_UI_LIST_FOOTER_TO_SCROLL_BAR_PADDING                                                                    \
    (VISION_UI_LIST_FOOTER_CENTER_TO_SCROLL_BAR_PADDING - VISION_UI_LIST_FOOTER_MAX_WIDTH / 2)
#define VISION_UI_LIST_TEXT_MAX_WIDTH                                                                                  \
    (VISION_UI_SCREEN_WIDTH - VISION_UI_LIST_FOOTER_TO_SCROLL_BAR_PADDING - VISION_UI_LIST_FOOTER_MAX_WIDTH -          \
     VISION_UI_LIST_FOOTER_TO_LEFT_PADDING - VISION_UI_LIST_HEADER_TO_TEXT_PADDING - VISION_UI_LIST_HEADER_MAX_WIDTH - \
     VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING)

#endif // VISION_UI_VISION_UI_CONFIG_H
