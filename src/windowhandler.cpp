#include "canvas.h"

namespace Canvas {
void WindowHandler::process_event(WindowCanvas& canvas, const Event& e) {
  if (e.index() == Events::WindowCloseEvent) {
    canvas.stop();
  }
}
void WindowHandler::on_update(WindowCanvas& canvas) {}

}  // namespace Canvas
