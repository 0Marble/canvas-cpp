#include "canvas.h"

namespace Canvas {

Canvas::Canvas(Viewport viewport) : viewport(viewport) {}

const Viewport& Canvas::get_viewport() const { return viewport; };
void Canvas::set_viewport(Viewport new_viewport) { viewport = new_viewport; };

void Canvas::add_line(float x1, float y1, float x2, float y2, Rgba color,
                      float thickness) {
  primitives.push_back(Line{
      .start = Vec2(x1, y1),
      .end = Vec2(x2, y2),
      .color = color,
      .thickness = thickness,
  });
}
void Canvas::add_circle(float x, float y, float radius, Rgba color) {
  primitives.push_back(
      Circle{.origin = Vec2(x, y), .radius = radius, .color = color});
}
void Canvas::add_triangle(Vec2 p1, Vec2 p2, Vec2 p3, Rgba color) {
  primitives.push_back(Triangle{.points = {p1, p2, p3}, .color = color});
}
void Canvas::clear_primitives() { primitives.clear(); }

void Canvas::update() {
  for (const auto& p : primitives) {
    switch (p.index()) {
      case 0:
        draw_primitive(std::get<0>(p));
        break;
      case 1:
        draw_primitive(std::get<1>(p));
        break;
      case 2:
        draw_primitive(std::get<2>(p));
        break;
      default:
        break;
    }
  }
}

Rgba Canvas::blend(const Rgba& top, const Rgba& bottom) const {
  Rgba out;
  out.a = top.a + bottom.a * (1.0 - top.a);
  out.r = (top.r * top.a + bottom.r * bottom.a * (1.0 - top.a)) / out.a;
  out.g = (top.g * top.a + bottom.g * bottom.a * (1.0 - top.a)) / out.a;
  out.b = (top.b * top.a + bottom.b * bottom.a * (1.0 - top.a)) / out.a;

  return out;
}

void Canvas::add_connected_points(
    const std::vector<std::pair<float, float>>& pts, Rgba color,
    float thickness) {
  if (pts.size() < 2) return;

  auto [prev_x, prev_y] = pts[0];
  for (size_t i = 1; i < pts.size(); i++) {
    auto [cur_x, cur_y] = pts[i];
    add_line(prev_x, prev_y, cur_x, cur_y, color, thickness);
    prev_x = cur_x;
    prev_y = cur_y;
  }
}

}  // namespace Canvas
