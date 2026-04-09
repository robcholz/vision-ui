## ESP32 Example

Lumen is a concrete ESP32 integration reference for Vision UI.
It uses Vision UI as the device UI layer inside an ESP32-C3 based desktop hardware node,
combining a 240x240 display, rotary encoder, buzzer, motion sensing, and power monitoring
into a long-running physical interface that can react to computer state, games, and system events.

In Lumen, Vision UI is used as the interactive front-end on top of the embedded runtime,
while the surrounding firmware and hardware stack handle sensing, control, and device-specific behavior.
That makes Lumen a good real-world example of how Vision UI can be embedded into an ESP32 product
rather than used only in a simulator.

Reference project:

- https://github.com/robcholz/Lumen/tree/main/components/vision-ui
