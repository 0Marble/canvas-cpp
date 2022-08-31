#include <cmath>
#include <iostream>

#include "canvas.h"

using namespace Canvas;

int main() {
  BmpCanvas img(
      500, 500, "canvas.bmp",
      Viewport{.top = 5.0, .bottom = -5.0, .left = -5.0, .right = 5.0});
  img.set_background(std::make_shared<GraphingPaperBackground>(0.05, 0.05));

  for (float t = 0.0; t <= 2.0 * M_PI; t += 0.1) {
    img.add_line(0.0, 0.0, std::cos(t), std::sin(t), RED, 0.0);
  }

  img.add_triangle(Vec2(-2.0, 2.0), Vec2(-2.0, 3.0), Vec2(-1.0, 2.5), BLUE);
  img.add_line(-2.0, -2.0, -3.0, 3.0,
               Rgba{.r = 1.0, .g = 0.0, .b = 0.0, .a = 0.5}, 0.1);

  img.update();
  img.display();
  return 0;
}