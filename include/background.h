
#ifndef __CANVAS_BACKGROUND_H
#define __CANVAS_BACKGROUND_H

#include "geometry.h"

namespace Canvas {

class Background {
 public:
  virtual Rgba color(float x, float y) = 0;
};

class SolidBackground : public Background {
 private:
  Rgba c = WHITE;

 public:
  SolidBackground();
  SolidBackground(Rgba c);
  virtual Rgba color(float x, float y) override;
};

class GraphingPaperBackground : public Background {
 private:
  float x_axis_step = 1.0, y_axis_step = 1.0;
  Rgba axis_color = BLACK, grid_color = GRAY, background_color = WHITE;
  float x_resolution, y_resolution;

 public:
  GraphingPaperBackground() = delete;
  GraphingPaperBackground(float x_resolution, float y_resolution);
  GraphingPaperBackground(float x_resolution, float y_resolution,
                          float x_axis_step, float y_axis_step, Rgba axis_color,
                          Rgba grid_color, Rgba background_color);
  virtual Rgba color(float x, float y) override;
};

}  // namespace Canvas

#endif