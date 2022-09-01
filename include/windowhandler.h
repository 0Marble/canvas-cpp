#ifndef __CANVAS_WINDOW_HANDLER_H
#define __CANVAS_WINDOW_HANDLER_H

#include <iostream>

#include "events.h"
#include "geometry.h"

namespace Canvas {

class WindowCanvas;

class WindowHandler {
 public:
  virtual void on_update(WindowCanvas& canvas);
  virtual void process_event(WindowCanvas& canvas, const Event& e);
};

}  // namespace Canvas

#endif