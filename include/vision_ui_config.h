//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_CONFIG_H
#define VISION_UI_VISION_UI_CONFIG_H

// screen config
#define VISION_UI_SCREEN_HEIGHT 240 // the width of the screen
#define VISION_UI_SCREEN_WIDTH 240 // the height of the screen

// system
#define VISION_UI_ALLOW_EXIT_BY_USER 0 // allow user to exit vision ui at the top level
#define VISION_UI_LIST_ROOT_ICON_VIEW 0 // if the root should be list view or icon view

#define VISION_UI_MAX_LIST_LAYER 10 // the max layer of the ui
#define VISION_UI_LIST_ROOT_CAPACITY 20 // the capacity of the root item

#define VISION_UI_EXIT_ANIMATION_DURATION_MS 180 // the time taken for the exit animation
#define VISION_UI_ENTER_ANIMATION_DURATION_MS 200 // the time taken for the enter animation

// notification
#define VISION_UI_NOTIFICATION_HEIGHT 15 // the height of the notification
#define VISION_UI_NOTIFICATION_WIDTH 20 // the width of the notification

#define VISION_UI_NOTIFICATION_DISMISS_DURATION_MS 1500 // the dismiss time of the notification

// alert
#define VISION_UI_ALERT_HEIGHT 20 // the height of the alert
#define VISION_UI_ALERT_WIDTH 20 // the width of the alert

// list view
#define VISION_UI_LIST_ENTRY_ANIMATION 0 // control whether items animate into place when a new list opens

#define VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING 0 // the vertical padding between the top display edge to the title
#define VISION_UI_LIST_TITLE_TO_FRAME_PADDING 4 // the vertical padding between the title to the item frame
#define VISION_UI_LIST_FRAME_BETWEEN_PADDING 2 // the vertical padding between each item frame in the list view
#define VISION_UI_LIST_FOOTER_CENTER_TO_SCROLL_BAR_PADDING 10 // the horizontal padding between the scroll bar and the center of the footer
#define VISION_UI_LIST_FOOTER_TO_LEFT_PADDING 10 // the min horizontal padding between the footer center and the right of the text area
#define VISION_UI_LIST_HEADER_TO_TEXT_PADDING 2 // the horizontal padding between the right of the header to the left of the text
#define VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING 4 // the horizontal padding between the left display edge to the header
#define VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING 3 // the padding between the edge of the selector to the inner widget

#define VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S 5 // the speed of scrolling of the slider value (if it slides!), in pixel/second
#define VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS 1500 // the time to pause when the scrolling text reaches both end of the scrolling area
#define VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS 1000 // the time to pause when the scrolling text reaches both end of the scrolling area
#define VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S 15 // the scrolling speed of text, in pixel per second

#define VISION_UI_LIST_FOOTER_MAX_HEIGHT 11 // the max height of the footer, this should be smaller than the frame height
#define VISION_UI_LIST_FOOTER_MAX_WIDTH 19 // the max width of the footer
#define VISION_UI_LIST_HEADER_MAX_HEIGHT 7 // the max height of the header, this should be smaller than the frame height
#define VISION_UI_LIST_HEADER_MAX_WIDTH 7 // the max width of the footer
#define VISION_UI_LIST_FRAME_FIXED_HEIGHT 15 // the fixed frame height
#define VISION_UI_LIST_SELECTOR_FIXED_HEIGHT 15 // the fixed height of the selector, this should typically equal to the frame height

// icon view
#define VISION_UI_ICON_VIEW_ITEM_SPACING 15 // the horizontal padding between the icon
#define VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING 0 // the horizontal padding from the title bar to the left edge of the display
#define VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING 4 // the horizontal padding from the right of the title bar to the right title text
#define VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING 5 // the min horizontal padding from the title text to the display right edge
#define VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING 2 // the vertical padding from the top display edge to the icon
#define VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING 2 // the vertical padding between the icon and the title area
#define VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING 3 // the vertical padding between the title area and the description area
#define VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING 3 // the min horizontal padding from each side of the display to description

#define VISION_UI_ICON_VIEW_SCROLL_SPEED 85 // the scrolling speed of the icon and the icon view title

#define VISION_UI_ICON_VIEW_ICON_SIZE 30 // the length of the side of the icon
#define VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT 13 // the height of the title area
#define VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH 4 // the width of the title bar

// ********** DO NOT EDIT LINES BELOW **********

// fixed properties
#define VISION_UI_LIST_SCROLL_BAR_WIDTH 3
#define VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED 92

// derived properties
#define VISION_UI_LIST_FOOTER_TO_SCROLL_BAR_PADDING                                                                                        \
    (VISION_UI_LIST_FOOTER_CENTER_TO_SCROLL_BAR_PADDING - VISION_UI_LIST_FOOTER_MAX_WIDTH / 2)
#define VISION_UI_LIST_TEXT_MAX_WIDTH                                                                                                      \
    (VISION_UI_SCREEN_WIDTH - VISION_UI_LIST_FOOTER_TO_SCROLL_BAR_PADDING - VISION_UI_LIST_FOOTER_MAX_WIDTH -                              \
     VISION_UI_LIST_FOOTER_TO_LEFT_PADDING - VISION_UI_LIST_HEADER_TO_TEXT_PADDING - VISION_UI_LIST_HEADER_MAX_WIDTH -                     \
     VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING)

#endif // VISION_UI_VISION_UI_CONFIG_H
