
#include "canvas.h"

using namespace Canvas;
int main() {
  GLFWCanvas img = GLFWCanvas(
      500, 500, "hello", std::make_shared<WindowHandler>(),
      Viewport{.top = 5.0, .bottom = -5.0, .left = -5.0, .right = 5.0});

  img.add_circle(0.0, 0.0, 1.0, BLUE);
  img.add_triangle(Vec2(-2.0, 1.0), Vec2(-1.0, 1.0), Vec2(-1.5, 2.0), RED);
  img.add_line(-1.0, 1.0, 1.0, -3.0, GREEN, 0.1);

  img.update();
  img.display();

  return 0;
}
