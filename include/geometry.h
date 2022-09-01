#ifndef __CANVAS_GEOMETRY_H
#define __CANVAS_GEOMETRY_H

#include <array>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

namespace Canvas {

class Vec2 {
 public:
  float x = 0.0f, y = 0.0f;
  Vec2(float x, float y);
  explicit Vec2(float t);
  virtual ~Vec2(){};

  Vec2 operator+(const Vec2& other) const;
  Vec2 operator-(const Vec2& other) const;
  Vec2 operator*(float t) const;
  Vec2 operator/(float t) const;

  static float dot(const Vec2& a, const Vec2& b);
  float len_squared() const;
  float len() const;
  Vec2 normalize() const;
};
std::ostream& operator<<(std::ostream& out, const Vec2& v);

struct Rgba {
  float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
};

bool operator==(const Rgba& a, const Rgba& b);

static constexpr Rgba NONE = {0.0, 0.0, 0.0, 0.0};
static constexpr Rgba WHITE = {1.0, 1.0, 1.0, 1.0};
static constexpr Rgba BLACK = {0.0, 0.0, 0.0, 1.0};
static constexpr Rgba RED = {1.0, 0.0, 0.0, 1.0};
static constexpr Rgba GREEN = {0.0, 1.0, 0.0, 1.0};
static constexpr Rgba BLUE = {0.0, 0.0, 1.0, 1.0};
static constexpr Rgba YELLOW = {1.0, 1.0, 0.0, 1.0};
static constexpr Rgba PURPLE = {1.0, 0.0, 1.0, 1.0};
static constexpr Rgba CYAN = {0.0, 1.0, 1.0, 1.0};
static constexpr Rgba GRAY = {0.5, 0.5, 0.5, 1.0};

struct Viewport {
  float top = 0.0f, bottom = 0.0f, left = 0.0f, right = 0.0f;
  bool contains(const Vec2& pt);

  static Vec2 convert(const Viewport& from, const Viewport& to, Vec2 pt);
  static float convert(const Viewport& from, const Viewport& to, float len);
};

struct Line {
  Vec2 start, end;
  Rgba color;
  float thickness;
};

struct Circle {
  Vec2 origin;
  float radius;
  Rgba color;
};

struct Triangle {
  std::array<Vec2, 3> points;
  Rgba color;
};

}  // namespace Canvas

#endif
