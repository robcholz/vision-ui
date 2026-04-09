# Vision UI Configuration Guide

This page explains how to tune [`include/vision_ui_config.h`](../include/vision_ui_config.h) without guessing what each
macro does.

If you only need the short version:

- Use compiler defines such as `-DCONFIG_VISION_UI_SCREEN_WIDTH=128` to override defaults.
- Start with screen size, then list row height, then spacing.
- Keep `VISION_UI_LIST_SELECTOR_FIXED_HEIGHT` equal to `VISION_UI_LIST_FRAME_FIXED_HEIGHT` unless you want the selector
  to look intentionally different.
- Treat the `DO NOT EDIT` block at the bottom of the header as internal.

## Layout Map

The sketch below matches the naming used in the config header:

![Vision UI layout sketch](layout.png)

Layout config diagram: [`layout-formal.drawio`](layout-formal.drawio)

Useful mental model:

- `DISPLAY` means the full screen.
- `FRAME` means one list row.
- `HEADER` is the small icon area on the left side of a row.
- `TEXT` is the main label area.
- `FOOTER` is the right-side widget area, such as the switch state or slider value.
- `SELECTOR` is the animated rounded rectangle around the current item.

## How to Override a Value

`include/vision_ui_config.h` reads `CONFIG_VISION_UI_*` macros and falls back to built-in defaults when a macro is not
provided.

The simplest override is a compiler define:

```bash
cc -DCONFIG_VISION_UI_SCREEN_WIDTH=128 -DCONFIG_VISION_UI_SCREEN_HEIGHT=64 ...
```

If you use xmake, add definitions to your target:

```lua
target("vision_ui")
    add_defines(
        "CONFIG_VISION_UI_SCREEN_WIDTH=128",
        "CONFIG_VISION_UI_SCREEN_HEIGHT=64",
        "CONFIG_VISION_UI_LIST_FRAME_FIXED_HEIGHT=13"
    )
```

If you use CMake, add compile definitions to the `vision_ui` target:

```cmake
target_compile_definitions(vision_ui PUBLIC
        CONFIG_VISION_UI_SCREEN_WIDTH=128
        CONFIG_VISION_UI_SCREEN_HEIGHT=64
        CONFIG_VISION_UI_LIST_FRAME_FIXED_HEIGHT=13
)
```

Why `vision_ui` and not only `vision_ui_simulator`:

- The macros change how the library sources are compiled.
- Adding them to `vision_ui` ensures the library and anything including its public headers see the same values.
- If you only add them to the final executable, the library itself may still be built with the defaults.

The pattern is always:

- define `CONFIG_VISION_UI_FOO`
- the library uses it as `VISION_UI_FOO`

## Recommended Tuning Order

Tune these groups in order:

1. Screen size
2. List row height and top spacing
3. Left and right spacing inside a row
4. Text scrolling speeds
5. Notifications and alerts
6. Icon view layout

This order matters because many other values only make visual sense after the screen and row height are stable.

## Screen and System

These values affect the entire UI:

| Macro                                   | Default | What it controls                                           |
|-----------------------------------------|---------|------------------------------------------------------------|
| `VISION_UI_SCREEN_HEIGHT`               | `240`   | Display height in pixels.                                  |
| `VISION_UI_SCREEN_WIDTH`                | `240`   | Display width in pixels.                                   |
| `VISION_UI_ALLOW_EXIT_BY_USER`          | `0`     | Whether the user can leave the UI from the top-level list. |
| `VISION_UI_MAX_LIST_LAYER`              | `10`    | Maximum nesting depth for list containers.                 |
| `VISION_UI_EXIT_ANIMATION_DURATION_MS`  | `180`   | Exit transition duration.                                  |
| `VISION_UI_ENTER_ANIMATION_DURATION_MS` | `200`   | Enter transition duration.                                 |

Notes:

- Set `SCREEN_WIDTH` and `SCREEN_HEIGHT` first. Almost every layout calculation depends on them.
- `MAX_LIST_LAYER` is a safety limit. Increase it only if your menu tree is genuinely deeper.
- The enter and exit durations change feel, not layout.

## Notifications and Alerts

These control the temporary overlays drawn above the current screen:

| Macro                                        | Default | What it controls                                         |
|----------------------------------------------|---------|----------------------------------------------------------|
| `VISION_UI_NOTIFICATION_HEIGHT`              | `15`    | Notification bar height.                                 |
| `VISION_UI_NOTIFICATION_WIDTH`               | `20`    | Extra horizontal padding added around notification text. |
| `VISION_UI_NOTIFICATION_DISMISS_DURATION_MS` | `1500`  | Notification lifetime before dismiss animation starts.   |
| `VISION_UI_ALERT_HEIGHT`                     | `20`    | Center alert height.                                     |
| `VISION_UI_ALERT_WIDTH`                      | `20`    | Extra horizontal padding added around alert text.        |

Practical guidance:

- If notification or alert text looks cramped, increase `*_WIDTH`.
- If those overlays feel too tall for a small screen, reduce `*_HEIGHT` first.
- `NOTIFICATION_DISMISS_DURATION_MS` changes how long a message stays visible, not how fast it animates.

## List View: Vertical Layout

These values control where rows sit vertically in the normal list view:

| Macro                                         | Default | What it controls                                      |
|-----------------------------------------------|---------|-------------------------------------------------------|
| `VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING` | `0`     | Top padding before the first title or first row.      |
| `VISION_UI_LIST_TITLE_TO_FRAME_PADDING`       | `4`     | Gap between a title row and the next selectable row.  |
| `VISION_UI_LIST_FRAME_BETWEEN_PADDING`        | `2`     | Gap between normal rows.                              |
| `VISION_UI_LIST_FRAME_FIXED_HEIGHT`           | `15`    | Height of each list row.                              |
| `VISION_UI_LIST_SELECTOR_FIXED_HEIGHT`        | `15`    | Height of the animated selector.                      |
| `VISION_UI_LIST_ENTRY_ANIMATION`              | `0`     | Whether rows animate into place when entering a list. |

How these work together:

- The first row starts at `TITLE_TO_DISPLAY_TOP_PADDING`.
- Every later row is positioned by adding `previous row Y + LIST_FRAME_FIXED_HEIGHT + gap`.
- The gap is `LIST_TITLE_TO_FRAME_PADDING` after a title row and `LIST_FRAME_BETWEEN_PADDING` after a normal row.

Recommended starting rules:

- Keep `LIST_SELECTOR_FIXED_HEIGHT == LIST_FRAME_FIXED_HEIGHT`.
- Increase `LIST_FRAME_FIXED_HEIGHT` before increasing inter-row gaps if the UI feels crowded.
- On short displays, reduce top and between-row spacing before shrinking text or icons.

## List View: Horizontal Layout

These are the values the `layout.png` sketch is mostly describing:

| Macro                                               | Default | What it controls                                                   |
|-----------------------------------------------------|---------|--------------------------------------------------------------------|
| `VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING`     | `4`     | Left margin from the screen edge to the row header icon area.      |
| `VISION_UI_LIST_HEADER_MAX_WIDTH`                   | `7`     | Reserved width for the header icon area.                           |
| `VISION_UI_LIST_HEADER_MAX_HEIGHT`                  | `7`     | Reserved height for the header icon area.                          |
| `VISION_UI_LIST_HEADER_TO_TEXT_PADDING`             | `2`     | Gap from the header icon area to the text area.                    |
| `VISION_UI_LIST_FOOTER_MAX_WIDTH`                   | `19`    | Reserved width for the right-side footer area for normal rows.     |
| `VISION_UI_LIST_FOOTER_MAX_HEIGHT`                  | `11`    | Reserved height for the footer area.                               |
| `VISION_UI_LIST_SLIDER_FOOTER_WIDTH`                | `10`    | Footer width used specifically by slider rows.                     |
| `VISION_UI_LIST_FOOTER_RIGHT_TO_SCROLL_BAR_PADDING` | `10`    | Gap between the footer area and the scrollbar.                     |
| `VISION_UI_LIST_FOOTER_TO_LEFT_PADDING`             | `10`    | Minimum space kept between the text area and the footer area.      |
| `VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING`   | `3`     | Inner padding between selector border and the header/text content. |

What this means on screen:

- Left side: `screen edge -> left display padding -> header -> header-to-text padding -> text`
- Right side: `text -> footer-to-left padding -> footer -> footer-right-to-scroll-bar padding -> scrollbar`

The text width is derived from those values and the screen width. You do not set text width directly.

If you see a specific problem, adjust this first:

- Header feels glued to the edge: increase `VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING`
- Text feels too close to the icon: increase `VISION_UI_LIST_HEADER_TO_TEXT_PADDING`
- Text collides with the right-side widget: increase `VISION_UI_LIST_FOOTER_TO_LEFT_PADDING`
- Right side feels too heavy: reduce `VISION_UI_LIST_FOOTER_MAX_WIDTH` or
  `VISION_UI_LIST_FOOTER_RIGHT_TO_SCROLL_BAR_PADDING`
- Selector looks cramped: increase `VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING`

Safety rules:

- Keep `VISION_UI_LIST_HEADER_MAX_HEIGHT` smaller than `VISION_UI_LIST_FRAME_FIXED_HEIGHT`
- Keep `VISION_UI_LIST_FOOTER_MAX_HEIGHT` smaller than `VISION_UI_LIST_FRAME_FIXED_HEIGHT`

## Scrolling and Motion

These values tune scrolling behavior in list rows:

| Macro                                           | Default | What it controls                                  |
|-------------------------------------------------|---------|---------------------------------------------------|
| `VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S`         | `15`    | Horizontal text scroll speed for long row labels. |
| `VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS`           | `1000`  | Pause when scrolling text reaches an end.         |
| `VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S` | `5`     | Scroll speed for long slider values.              |
| `VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS`   | `1500`  | Pause when a slider value reaches an end.         |

Practical guidance:

- Raise the speed if long labels feel slow to inspect.
- Raise the pause if long labels feel twitchy or hard to read.
- Slider values usually need a slower speed than titles because they are more glance-driven.

## Icon View

These values control the alternate icon-based screen layout:

| Macro                                                    | Default | What it controls                                        |
|----------------------------------------------------------|---------|---------------------------------------------------------|
| `VISION_UI_ICON_VIEW_ITEM_SPACING`                       | `15`    | Horizontal spacing between icons.                       |
| `VISION_UI_ICON_VIEW_ICON_SIZE`                          | `100`   | Icon box size in pixels.                                |
| `VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING`        | `5`     | Top margin before the icon area.                        |
| `VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING`         | `10`    | Gap between the icon and the title area below it.       |
| `VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT`                  | `70`    | Height reserved for the title area.                     |
| `VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING`  | `0`     | Left margin before the vertical title bar.              |
| `VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH`                    | `9`     | Width of the title bar.                                 |
| `VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING`         | `10`    | Gap between the title bar and the title text.           |
| `VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING` | `15`    | Minimum right margin for the title text area.           |
| `VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING`  | `20`    | Gap between the title block and description text.       |
| `VISION_UI_ICON_VIEW_DESCRIPTION_AREA_HEIGHT`            | `35`    | Height reserved for the description area.               |
| `VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING` | `3`     | Minimum left and right margin for the description text. |
| `VISION_UI_ICON_VIEW_SCROLL_SPEED`                       | `85`    | Animation speed when moving between icons.              |

Recommended approach:

- Start by fitting `ICON_SIZE` to your display.
- Then tune the vertical paddings and title area height.
- Only after that adjust description spacing and scroll speed.

Quick checks:

- Icon view looks vertically cramped: reduce `ICON_SIZE`, `TITLE_AREA_HEIGHT`, or `TITLE_AREA_TO_DESCRIPTION_PADDING`
- Title text is too boxed in: increase `TITLE_BAR_TO_TITLE_PADDING` or `TITLE_TO_RIGHT_DISPLAY_MIN_PADDING`
- Description text hugs the edge: increase `DESCRIPTION_TO_DISPLAY_MIN_SPACING`

## Values You Should Usually Leave Alone

At the end of the config header there is a block marked `DO NOT EDIT`.

Those values are internal assumptions:

- `VISION_UI_LIST_SCROLL_BAR_WIDTH`
- `VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED`
- `VISION_UI_LIST_TEXT_MAX_WIDTH(currentListItem)`

In particular, `VISION_UI_LIST_TEXT_MAX_WIDTH(...)` is derived from screen width plus the left and right layout
reservations. If text width looks wrong, fix the spacing inputs above instead of editing the macro directly.

## Example Profiles

### Small 128x64 display

```c
CONFIG_VISION_UI_SCREEN_WIDTH=128
CONFIG_VISION_UI_SCREEN_HEIGHT=64
CONFIG_VISION_UI_LIST_FRAME_FIXED_HEIGHT=13
CONFIG_VISION_UI_LIST_SELECTOR_FIXED_HEIGHT=13
CONFIG_VISION_UI_LIST_FOOTER_RIGHT_TO_SCROLL_BAR_PADDING=6
CONFIG_VISION_UI_LIST_FOOTER_TO_LEFT_PADDING=6
```

### More spacious 240x240 display

The defaults are already tuned for this case. Usually only text speed and icon-view spacing need adjustment.

## Troubleshooting

If the UI looks wrong, use this checklist:

- Text overlaps the footer: increase `VISION_UI_LIST_FOOTER_TO_LEFT_PADDING`, reduce `VISION_UI_LIST_FOOTER_MAX_WIDTH`,
  or reduce your font width
- The selector is taller or shorter than the row: make `VISION_UI_LIST_SELECTOR_FIXED_HEIGHT` match
  `VISION_UI_LIST_FRAME_FIXED_HEIGHT`
- The first row is clipped at the top: increase `VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING`
- Too few rows fit on screen: reduce `VISION_UI_LIST_FRAME_FIXED_HEIGHT`, `VISION_UI_LIST_TITLE_TO_FRAME_PADDING`, or
  `VISION_UI_LIST_FRAME_BETWEEN_PADDING`
- Icon view title or description gets squeezed: reduce `VISION_UI_ICON_VIEW_ICON_SIZE` or shrink one of the vertical
  paddings

## Related Docs

- API reference: [`docs/api.md`](api.md)
- Main project guide: [`README.md`](../README.md)
- Config header source: [`include/vision_ui_config.h`](../include/vision_ui_config.h)
