#include "canvas.h"

namespace Canvas {
SolidBackground::SolidBackground(Rgba c) : c(c) {}
SolidBackground::SolidBackground() : c(WHITE) {}
Rgba SolidBackground::color(float x, float y) { return c; }

GraphingPaperBackground::GraphingPaperBackground(float x_resolution,
                                                 float y_resolution)
    : x_axis_step(1.0),
      y_axis_step(1.0),
      axis_color(BLACK),
      grid_color(GRAY),
      background_color(WHITE),
      x_resolution(x_resolution),
      y_resolution(y_resolution) {}
GraphingPaperBackground::GraphingPaperBackground(
    float x_resolution, float y_resolution, float x_axis_step,
    float y_axis_step, Rgba axis_color, Rgba grind_color, Rgba background_color)
    : x_axis_step(x_axis_step),
      y_axis_step(y_axis_step),
      axis_color(axis_color),
      grid_color(grid_color),
      background_color(background_color) {}

Rgba GraphingPaperBackground::color(float x, float y) {
  if (std::abs(x) < x_resolution || std::abs(y) < y_resolution)
    return axis_color;
  if (std::abs((x / x_axis_step) - int64_t(x / x_axis_step)) < x_resolution ||
      std::abs((y / y_axis_step) - int64_t(y / y_axis_step)) < y_resolution)
    return grid_color;
  return background_color;
}

}  // namespace Canvas
