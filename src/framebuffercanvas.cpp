#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <ranges>
#include <stack>
#include <tuple>

#include "canvas.h"

using namespace Canvas;
static std::optional<float> intersect_lines(const Vec2& p1, const Vec2& d1,
                                            const Vec2& p2, const Vec2& d2) {
  // d1x * t1 - d2x * t2 = p2x - p1x
  // d1y * t1 - d2y * t2 = p2y - p1y
  //
  // d2y * d1x * t1 - d2y * d2x * t2 = d2y * (p2x - p1x)
  // d2x * d1y * t1 - d2y * d2x * t2 = d2x * (p2y - p1y)
  //
  // t1 * (d2y * d1x - d2x * d1y) = d2y * (p2x - p1x) - d2x * (p2y - p1y)

  float div = d2.y * d1.x - d2.x * d1.y;
  if (div == 0.0f) return {};
  div = 1.0f / div;

  return div * d2.y * (p2.x - p1.x) - div * d2.x * (p2.y - p1.y);
}

static std::optional<Line> bound_line(const Line& l, const Viewport& v) {
  // bd
  // ac
  Vec2 a(v.left, v.bottom), b(v.left, v.top), c(v.right, v.bottom),
      d(v.right, v.top);

  Vec2 p(l.start), dir = Vec2(l.end - l.start);
  if (dir.x == 0.0 && dir.y == 0.0) return {};
  auto t1_res = intersect_lines(p, dir, a, c - a);
  auto t2_res = intersect_lines(p, dir, a, b - a);
  auto t3_res = intersect_lines(p, dir, b, d - b);
  auto t4_res = intersect_lines(p, dir, c, d - c);
  float t1 = 0.0f, t2 = 0.0f, t3 = 0.0f, t4 = 0.0f;

  if (!t1_res.has_value()) {
    if (p.x < v.left || p.x > v.right) return {};
    UNWRAP(t2_res, t2);
    UNWRAP(t4_res, t4);
  } else if (!t2_res.has_value()) {
    if (p.y < v.bottom || p.y > v.top) return {};
    UNWRAP(t1_res, t1);
    UNWRAP(t3_res, t3);
  } else {
    UNWRAP(t1_res, t1);
    UNWRAP(t2_res, t2);
    UNWRAP(t3_res, t3);
    UNWRAP(t4_res, t4);
  }

  float t_min = std::clamp(std::min({t1, t2, t3, t4, 0.0f, 1.0f}), 0.0f, 1.0f);
  float t_max = std::clamp(std::max({t1, t2, t3, t4, 0.0f, 1.0f}), 0.0f, 1.0f);
  return Line{.start = p + dir * t_min,
              .end = p + dir * t_max,
              .color = l.color,
              .thickness = l.thickness};
}

namespace Canvas {

FrameBufferCanvas::FrameBufferCanvas(uint32_t width, uint32_t height,
                                     Viewport viewport)
    : Canvas(viewport), width(width), height(height) {}

Viewport FrameBufferCanvas::pixel_viewport() const {
  return Viewport{
      .top = float(height - 1),
      .bottom = 0.0,
      .left = 0.0,
      .right = float(width - 1),
  };
}

std::optional<Rgba> FrameBufferCanvas::sample(float x, float y) const {
  Vec2 pixel_coords = Viewport::convert(viewport, pixel_viewport(), Vec2(x, y));

  if (pixel_coords.x < 0.0 || pixel_coords.x >= width || pixel_coords.y < 0.0 ||
      pixel_coords.y >= height)
    return {};

  return get_pixel(pixel_coords.x, pixel_coords.y);
}

void FrameBufferCanvas::blend_pixel(uint32_t x, uint32_t y, Rgba color) {
  set_pixel(x, y, blend(color, get_pixel(x, y)));
}

void FrameBufferCanvas::draw_background() {
  for (uint32_t x = 0; x < width; x++) {
    for (uint32_t y = 0; y < height; y++) {
      Vec2 world_coords =
          Viewport::convert(pixel_viewport(), viewport, Vec2(x, y));
      if (background.has_value()) {
        set_pixel(x, y,
                  background.value()->color(world_coords.x, world_coords.y));
      } else {
        set_pixel(x, y, NONE);
      }
    }
  }
}

void FrameBufferCanvas::floodfill(float x, float y, Rgba color) {
  Vec2 pixel = Viewport::convert(viewport, pixel_viewport(), Vec2(x, y));
  floodfill(uint32_t(pixel.x), uint32_t(pixel.y), color);
}

void FrameBufferCanvas::floodfill(uint32_t x, uint32_t y, Rgba color) {
  if (x >= width || y >= height) return;

  std::stack<std::tuple<uint32_t, uint32_t>> points;
  points.push(std::make_tuple(x, y));
  auto c = get_pixel(x, y);

  while (!points.empty()) {
    auto [px, py] = points.top();
    uint32_t x = px, y = py;
    points.pop();

    set_pixel(x, y, color);

    if (x + 1 < width && c == get_pixel(x + 1, y)) {
      set_pixel(x + 1, y, color);
      points.push(std::make_tuple(x + 1, y));
    }

    if (x > 0 && c == get_pixel(x - 1, y)) {
      set_pixel(x - 1, y, color);
      points.push(std::make_tuple(x - 1, y));
    }

    if (y + 1 < height && c == get_pixel(x, y + 1)) {
      set_pixel(x, y + 1, color);
      points.push(std::make_tuple(x, y + 1));
    }

    if (y > 0 && c == get_pixel(x, y - 1)) {
      set_pixel(x, y - 1, color);
      points.push(std::make_tuple(x, y - 1));
    }
  }
}

void FrameBufferCanvas::blit_canvas(const Canvas& other, Viewport location) {
  Vec2 pixel_p1 = Viewport::convert(viewport, pixel_viewport(),
                                    Vec2(location.left, location.bottom));
  Vec2 pixel_p2 = Viewport::convert(viewport, pixel_viewport(),
                                    Vec2(location.right, location.top));

  uint32_t min_x = std::clamp(pixel_p1.x, 0.0f, float(width - 1));
  uint32_t max_x = std::clamp(pixel_p2.x, 0.0f, float(width - 1));
  uint32_t min_y = std::clamp(pixel_p1.y, 0.0f, float(height - 1));
  uint32_t max_y = std::clamp(pixel_p2.y, 0.0f, float(height - 1));

  Viewport partial_pixel_viewport = {.top = float(max_y),
                                     .bottom = float(min_y),
                                     .left = float(min_x),
                                     .right = float(max_x)};

  for (uint32_t x = min_x; x <= max_x; x++) {
    for (uint32_t y = min_y; y <= max_y; y++) {
      Vec2 other_point = Viewport::convert(partial_pixel_viewport,
                                           other.get_viewport(), Vec2(x, y));
      Rgba other_color =
          other.sample(other_point.x, other_point.y).value_or(Rgba());
      blend_pixel(x, y, other_color);
    }
  }
}

void FrameBufferCanvas::blit_canvas(const FrameBufferCanvas& other,
                                    Viewport location) {
  Vec2 pixel_p1 = Viewport::convert(viewport, pixel_viewport(),
                                    Vec2(location.left, location.bottom));
  Vec2 pixel_p2 = Viewport::convert(viewport, pixel_viewport(),
                                    Vec2(location.right, location.top));

  uint32_t min_x = std::clamp(pixel_p1.x, 0.0f, float(width - 1));
  uint32_t max_x = std::clamp(pixel_p2.x, 0.0f, float(width - 1));
  uint32_t min_y = std::clamp(pixel_p1.y, 0.0f, float(height - 1));
  uint32_t max_y = std::clamp(pixel_p2.y, 0.0f, float(height - 1));

  Viewport partial_pixel_viewport = {.top = float(max_y),
                                     .bottom = float(min_y),
                                     .left = float(min_x),
                                     .right = float(max_x)};

  for (uint32_t x = min_x; x <= max_x; x++) {
    for (uint32_t y = min_y; y <= max_y; y++) {
      Vec2 other_point = Viewport::convert(partial_pixel_viewport,
                                           other.pixel_viewport(), Vec2(x, y));
      Rgba other_color = other.get_pixel(other_point.x, other_point.y);
      blend_pixel(x, y, other_color);
    }
  }
}

void FrameBufferCanvas::draw_primitive(const Triangle& p) {
  if (p.color == NONE) return;

  std::array<Vec2, 3> points = {
      Viewport::convert(viewport, pixel_viewport(), p.points[0]),
      Viewport::convert(viewport, pixel_viewport(), p.points[1]),
      Viewport::convert(viewport, pixel_viewport(), p.points[2])};
  for (auto& p : points) p = Vec2(int64_t(p.x), int64_t(p.y));

  Viewport tri_pixels = {
      .top = std::max({
          points[0].y,
          points[1].y,
          points[2].y,
      }),
      .bottom = std::min({
          points[0].y,
          points[1].y,
          points[2].y,
      }),
      .left = std::min({
          points[0].x,
          points[1].x,
          points[2].x,
      }),
      .right = std::max({
          points[0].x,
          points[1].x,
          points[2].x,
      }),
  };

  BmpCanvas tri =
      BmpCanvas(tri_pixels.right - tri_pixels.left + 1,
                tri_pixels.top - tri_pixels.bottom + 1, "", tri_pixels);

  tri.draw_pixel_triangle(
      points[0].x - tri_pixels.left, points[0].y - tri_pixels.bottom,
      points[1].x - tri_pixels.left, points[1].y - tri_pixels.bottom,
      points[2].x - tri_pixels.left, points[2].y - tri_pixels.bottom, p.color);
  // tri.draw_pixel_line(
  //     points[0].x - tri_pixels.left, points[0].y - tri_pixels.bottom,
  //     points[1].x - tri_pixels.left, points[1].y - tri_pixels.bottom,
  //     p.color);
  // tri.draw_pixel_line(
  //     points[1].x - tri_pixels.left, points[1].y - tri_pixels.bottom,
  //     points[2].x - tri_pixels.left, points[2].y - tri_pixels.bottom,
  //     p.color);
  // tri.draw_pixel_line(
  //     points[2].x - tri_pixels.left, points[2].y - tri_pixels.bottom,
  //     points[0].x - tri_pixels.left, points[0].y - tri_pixels.bottom,
  //     p.color);

  for (uint32_t x = 0; x < tri.width; x++) {
    for (uint32_t y = 0; y < tri.height; y++) {
      Rgba c = tri.get_pixel(x, y);
      if (pixel_viewport().contains(
              Vec2(x + tri_pixels.left, y + tri_pixels.bottom))) {
        blend_pixel(x + tri_pixels.left, y + tri_pixels.bottom, c);
      }
    }
  }
}

void FrameBufferCanvas::draw_primitive(const Line& l) {
  if (l.thickness == 0.0f) {
    auto bounded_line_res = bound_line(l, viewport);
    if (!bounded_line_res.has_value()) return;
    Line bounded_line = bounded_line_res.value();

    Vec2 a = Viewport::convert(viewport, pixel_viewport(), bounded_line.start);
    Vec2 b = Viewport::convert(viewport, pixel_viewport(), bounded_line.end);

    draw_pixel_line(a.x, a.y, b.x, b.y, l.color);

  } else {
    Vec2 a = l.start, b = l.end;
    Vec2 dir = b - a;
    Vec2 c = a + Vec2(-dir.y, dir.x).normalize() * l.thickness;
    Vec2 d = a + a - c;
    Vec2 e = d + (b - a), f = c + (b - a);

    draw_primitive(Triangle{.points = {c, d, e}, .color = l.color});
    draw_primitive(Triangle{.points = {c, f, e}, .color = l.color});

    draw_primitive(
        Circle{.origin = a, .radius = l.thickness, .color = l.color});
    draw_primitive(
        Circle{.origin = b, .radius = l.thickness, .color = l.color});
  }
}

void FrameBufferCanvas::draw_primitive(const Circle& c) {
  Vec2 prev = Vec2(std::cos(0.0f), std::sin(0.0f)) * c.radius + c.origin;
  float step = 0.1;

  for (float t = step; t <= 2.0 * M_PI; t += step) {
    Vec2 cur = Vec2(std::cos(t), std::sin(t)) * c.radius + c.origin;
    draw_primitive(Triangle{.points = {c.origin, prev, cur}, .color = c.color});
  }
}

void FrameBufferCanvas::draw_pixel_line(uint32_t x1, uint32_t y1, uint32_t x2,
                                        uint32_t y2, Rgba color) {
  int64_t dx = std::abs(int64_t(x1) - int64_t(x2));
  int64_t sx = (x1 > x2) ? -1 : +1;
  int64_t dy = -std::abs(int64_t(y1) - int64_t(y2));
  int64_t sy = (y1 > y2) ? -1 : +1;
  int64_t error = dx + dy;

  while (!draw_pixel_line_step(x1, y1, x2, y2, dx, dy, sx, sy, error, color))
    ;
}

bool FrameBufferCanvas::draw_pixel_line_step(uint32_t& x1, uint32_t& y1,
                                             uint32_t x2, uint32_t y2,
                                             int64_t dx, int64_t dy, int64_t sx,
                                             int64_t sy, int64_t& error,
                                             Rgba color) {
  blend_pixel(x1, y1, color);
  if (x1 == x2 && y1 == y2) return true;
  int64_t e2 = 2 * error;
  if (e2 >= dy) {
    if (x1 == x2) return true;
    error += dy;
    x1 += sx;
  }
  if (e2 <= dx) {
    if (y1 == y2) return true;
    error += dx;
    y1 += sy;
  }

  return false;
}

void FrameBufferCanvas::draw_pixel_triangle(uint32_t x1, uint32_t y1,
                                            uint32_t x2, uint32_t y2,
                                            uint32_t x3, uint32_t y3,
                                            Rgba color) {
  std::array<std ::pair<uint32_t, uint32_t>, 3> verts = {
      std::make_pair(x1, y1),
      std::make_pair(x2, y2),
      std::make_pair(x3, y3),
  };
  std::sort(verts.begin(), verts.end(),
            [](const std::pair<uint32_t, uint32_t>& a,
               const std::pair<uint32_t, uint32_t>& b) {
              return (a.second > b.second);
            });

  if (verts[1].second == verts[2].second) {
    draw_flat_bottom_pixel_triangle(std::min(verts[1].first, verts[2].first),
                                    std::max(verts[1].first, verts[2].first),
                                    verts[2].second, verts[0].first,
                                    verts[0].second, color);
  } else if (verts[0].second == verts[1].second) {
    draw_flat_top_pixel_triangle(std::min(verts[1].first, verts[0].first),
                                 std::max(verts[1].first, verts[0].first),
                                 verts[0].second, verts[2].first,
                                 verts[2].second, color);
  } else {
    uint32_t x3 = verts[0].first +
                  ((float(verts[1].second) - float(verts[0].second)) /
                   (float(verts[2].second) - float(verts[0].second))) *
                      (float(verts[2].first) - float(verts[0].first)),
             y3 = verts[1].second;
    draw_flat_bottom_pixel_triangle(std::min(verts[1].first, x3),
                                    std::max(x3, verts[1].first), y3,
                                    verts[0].first, verts[0].second, color);
    draw_flat_top_pixel_triangle(std::min(verts[1].first, x3),
                                 std::max(x3, verts[1].first), y3,
                                 verts[2].first, verts[2].second, color);
  }
}

void FrameBufferCanvas::draw_flat_top_pixel_triangle(
    uint32_t left_x, uint32_t right_x, uint32_t top_y, uint32_t bottom_x,
    uint32_t bottom_y, Rgba color) {
  int64_t left_dx = std::abs(int64_t(left_x) - int64_t(bottom_x)),
          right_dx = std::abs(int64_t(right_x) - int64_t(bottom_x));
  int64_t left_sx = (left_x > bottom_x) ? +1 : -1,
          right_sx = (right_x > bottom_x) ? +1 : -1;
  int64_t dy = -std::abs(int64_t(top_y) - int64_t(bottom_y));
  int64_t sy = (top_y > bottom_y) ? +1 : -1;
  int64_t left_error = left_dx + dy, right_error = right_dx + dy;
  uint32_t left_bottom_x = bottom_x, right_bottom_x = bottom_x,
           left_bottom_y = bottom_y, right_bottom_y = bottom_y;

  while (true) {
    bool left_finished = false, right_finished = false;

    uint32_t old_left_y = left_bottom_y, old_right_y = right_bottom_y;

    while (!left_finished) {
      left_finished =
          draw_pixel_line_step(left_bottom_x, left_bottom_y, left_x, top_y,
                               left_dx, dy, left_sx, sy, left_error, color);
      if (old_left_y != left_bottom_y) break;
    }

    while (!right_finished) {
      right_finished =
          draw_pixel_line_step(right_bottom_x, right_bottom_y, right_x, top_y,
                               right_dx, dy, right_sx, sy, right_error, color);
      if (old_right_y != right_bottom_y) break;
    }

    for (uint32_t x = left_bottom_x; x <= right_bottom_x; x++) {
      blend_pixel(x, right_bottom_y, color);
    }

    if (left_finished && right_finished) break;
  }
}

void FrameBufferCanvas::draw_flat_bottom_pixel_triangle(
    uint32_t left_x, uint32_t right_x, uint32_t bottom_y, uint32_t top_x,
    uint32_t top_y, Rgba color) {
  int64_t left_dx = std::abs(int64_t(left_x) - int64_t(top_x)),
          right_dx = std::abs(int64_t(right_x) - int64_t(top_x));
  int64_t left_sx = (left_x > top_x) ? +1 : -1,
          right_sx = (right_x > top_x) ? +1 : -1;
  int64_t dy = -std::abs(int64_t(top_y) - int64_t(bottom_y));
  int64_t sy = (top_y > bottom_y) ? -1 : +1;
  int64_t left_error = left_dx + dy, right_error = right_dx + dy;
  uint32_t left_top_x = top_x, right_top_x = top_x, left_top_y = top_y,
           right_top_y = top_y;

  while (true) {
    bool left_finished = false, right_finished = false;
    uint32_t old_left_y = left_top_y, old_right_y = right_top_y;

    while (!left_finished) {
      left_finished =
          draw_pixel_line_step(left_top_x, left_top_y, left_x, bottom_y,
                               left_dx, dy, left_sx, sy, left_error, color);
      if (old_left_y != left_top_y) break;
    }

    while (!right_finished) {
      right_finished =
          draw_pixel_line_step(right_top_x, right_top_y, right_x, bottom_y,
                               right_dx, dy, right_sx, sy, right_error, color);
      if (old_right_y != right_top_y) break;
    }

    for (uint32_t x = left_top_x; x <= right_top_x; x++) {
      blend_pixel(x, right_top_y, color);
    }

    if (left_finished && right_finished) break;
  }
}

}  // namespace Canvas