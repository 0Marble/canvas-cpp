#include <cmath>

#include "canvas.h"

namespace Canvas {

Vec2::Vec2(float x, float y) : x(x), y(y) {}
Vec2::Vec2(float t) : x(t), y(t) {}

Vec2 Vec2::operator+(const Vec2& other) const {
  return Vec2(x + other.x, y + other.y);
}

Vec2 Vec2::operator-(const Vec2& other) const {
  return Vec2(x - other.x, y - other.y);
}

Vec2 Vec2::operator*(float t) const { return Vec2(x * t, y * t); }
Vec2 Vec2::operator/(float t) const { return Vec2(x / t, y / t); }

float Vec2::dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }

float Vec2::len_squared() const { return Vec2::dot(*this, *this); }
float Vec2::len() const { return std::sqrt(Vec2::dot(*this, *this)); }
Vec2 Vec2::normalize() const { return *this / len(); }

std::ostream& operator<<(std::ostream& out, const Vec2& v) {
  out << "(" << v.x << ", " << v.y << ")";
  return out;
}

Vec2 Viewport::convert(const Viewport& from, const Viewport& to, Vec2 pt) {
  float u = (pt.x - from.left) / (from.right - from.left);
  float v = (pt.y - from.bottom) / (from.top - from.bottom);

  return Vec2(u * (to.right - to.left) + to.left,
              v * (to.top - to.bottom) + to.bottom);
}

float Viewport::convert(const Viewport& from, const Viewport& to, float len) {
  float u = len / (from.right - from.left);
  float v = len / (from.top - from.bottom);

  return (u * (to.right - to.left) + v * (to.top - to.bottom)) * 0.5;
}

bool Viewport::contains(const Vec2& pt) {
  return (pt.x <= right && pt.x >= left && pt.y <= top && pt.y >= bottom);
}

bool operator==(const Rgba& a, const Rgba& b) {
  return a.a == b.a && a.r == b.r && a.g == b.g && a.b == b.b;
}

}  // namespace Canvas
