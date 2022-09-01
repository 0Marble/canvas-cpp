#include "canvas.h"

namespace Canvas {

WindowCanvas::WindowCanvas(std::shared_ptr<WindowHandler>&& handler,
                           Viewport viewport)
    : Canvas(viewport), handler(handler) {}

void WindowCanvas::stop() { quit = true; }

void WindowCanvas::update() {
  Canvas::update();
  while (true) {
    auto event = next_event();
    if (!event.has_value()) break;
    handler->process_event(*this, event.value());
  }

  handler->on_update(*this);
}

bool WindowCanvas::has_quit() const { return quit; }

}  // namespace Canvas
