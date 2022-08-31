#ifndef __CANVAS_EVENT_H
#define __CANVAS_EVENT_H

#include <iostream>
#include <variant>

namespace Canvas {

namespace Events {
static constexpr uint8_t MouseDownEvent = 0, MouseUpEvent = 1,
                         MouseWheelEvent = 2, MouseMoveEvent = 3,
                         KeydownEvent = 4, KeyupEvent = 5,
                         WindowResizeEvent = 6, WindowCloseEvent = 7;

}

enum class MouseButton : uint8_t { LEFT, RIGHT, MIDDLE };

struct MouseDownEvent {
  MouseButton button;
  uint32_t x, y;
};

struct MouseUpEvent {
  MouseButton button;
  uint32_t x, y;
};

struct MouseWheelEvent {
  bool direction;
  uint32_t dist;
};

struct MouseMoveEvent {
  uint32_t x, y;
};

enum class KeyCode {};

struct KeydownEvent {
  KeyCode key;
};

struct KeyupEvent {
  KeyCode key;
};

struct WindowResizeEvent {
  uint32_t new_width, new_height;
};

struct WindowCloseEvent {};

using Event =
    std::variant<MouseDownEvent, MouseUpEvent, MouseWheelEvent, MouseMoveEvent,
                 KeydownEvent, KeyupEvent, WindowResizeEvent, WindowCloseEvent>;

}  // namespace Canvas

#endif