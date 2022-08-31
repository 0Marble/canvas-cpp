#ifndef __CANVAS_CANVAS_H
#define __CANVAS_CANVAS_H

#include <array>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "background.h"
#include "geometry.h"
#include "windowhandler.h"

#define CANVAS_DEBUG

#ifdef CANVAS_DEBUG
#define INDEX_GET(res, vec, index)                                   \
  {                                                                  \
    size_t computed_index = y * width + x;                           \
    if (computed_index >= pixels.size()) {                           \
      std::cerr << __func__ << "@" __FILE__ ":" << __LINE__          \
                << " About to segfault! Index is " << computed_index \
                << ", size is " << pixels.size() << "\n";            \
      __builtin_trap();                                              \
    }                                                                \
    res = pixels[computed_index];                                    \
  }

#define INDEX_SET(vec, index, value)                                 \
  {                                                                  \
    size_t computed_index = y * width + x;                           \
    if (computed_index >= pixels.size()) {                           \
      std::cerr << __func__ << "@" __FILE__ ":" << __LINE__          \
                << " About to segfault! Index is " << computed_index \
                << ", size is " << pixels.size() << "\n";            \
      __builtin_trap();                                              \
    }                                                                \
    pixels[computed_index] = value;                                  \
  }

#define UNWRAP(var, into)                                 \
  if (!var.has_value()) {                                 \
    std::cerr << __func__ << "@" __FILE__ ":" << __LINE__ \
              << " Unwrapping an empty variant\n";        \
    __builtin_trap();                                     \
  } else {                                                \
    into = var.value();                                   \
  }

#else
#define INDEX_GET(reciever, vec, index) reciever = vec[index];
#define INDEX_SET(vec, index, value) vec[index] = value;
#define UNWRAP(var, into) into = var.value();
#endif

namespace Canvas {

class Canvas {
 protected:
  std::list<std::variant<Line, Circle, Triangle>> primitives;
  std::optional<std::shared_ptr<Background>> background;
  Viewport viewport;

  virtual void draw_primitive(const Line& l) = 0;
  virtual void draw_primitive(const Circle& c) = 0;
  virtual void draw_primitive(const Triangle& p) = 0;
  virtual void draw_background() = 0;

  virtual Rgba blend(const Rgba& top, const Rgba& bottom) const;

 public:
  Canvas() = delete;
  Canvas(Viewport viewport);
  virtual ~Canvas(){};

  virtual const Viewport& get_viewport() const;
  virtual void set_viewport(Viewport new_viewport);
  virtual void set_background(std::shared_ptr<Background>&& new_background);
  virtual void clear_background();

  virtual std::optional<Rgba> sample(float x, float y) const = 0;

  virtual void add_line(float x1, float y1, float x2, float y2, Rgba color,
                        float thickness);
  virtual void add_circle(float x, float y, float radius, Rgba color);
  virtual void add_triangle(Vec2 p1, Vec2 p2, Vec2 p3, Rgba color);
  virtual void clear_primitives();

  virtual void add_connected_points(
      const std::vector<std::pair<float, float>>& pts, Rgba color,
      float thickness);

  virtual void blit_canvas(const Canvas& other, Viewport location) = 0;

  virtual void update();
  virtual void display() = 0;
};

class FrameBufferCanvas : public Canvas {
 protected:
  uint32_t width, height;

  virtual void draw_primitive(const Line& l) override;
  virtual void draw_primitive(const Circle& c) override;
  virtual void draw_primitive(const Triangle& p) override;
  virtual void draw_background() override;

  Viewport pixel_viewport() const;
  void draw_pixel_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
                       Rgba color);

  bool draw_pixel_line_step(uint32_t& x1, uint32_t& y1, uint32_t x2,
                            uint32_t y2, int64_t dx, int64_t dy, int64_t sx,
                            int64_t sy, int64_t& error, Rgba color);

  void draw_pixel_triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
                           uint32_t x3, uint32_t y3, Rgba color);
  void draw_flat_top_pixel_triangle(uint32_t left_x, uint32_t right_x,
                                    uint32_t top_y, uint32_t bottom_x,
                                    uint32_t bottom_y, Rgba color);
  void draw_flat_bottom_pixel_triangle(uint32_t left_x, uint32_t right_x,
                                       uint32_t bottom_y, uint32_t top_x,
                                       uint32_t top_y, Rgba color);

 public:
  FrameBufferCanvas() = delete;
  FrameBufferCanvas(uint32_t width, uint32_t height, Viewport viewport);
  virtual ~FrameBufferCanvas(){};

  virtual std::optional<Rgba> sample(float x, float y) const override;
  virtual void blend_pixel(uint32_t x, uint32_t y, Rgba color);
  virtual Rgba get_pixel(uint32_t x, uint32_t y) const = 0;
  virtual void set_pixel(uint32_t x, uint32_t y, Rgba color) = 0;

  virtual void floodfill(uint32_t x, uint32_t y, Rgba color);
  virtual void floodfill(float x, float y, Rgba color);

  virtual void blit_canvas(const Canvas& other, Viewport location) override;
  virtual void blit_canvas(const FrameBufferCanvas& other, Viewport location);
};

class BmpCanvas : public FrameBufferCanvas {
 protected:
  std::string file_path;
  std::vector<Rgba> pixels;

 public:
  BmpCanvas() = delete;
  BmpCanvas(uint32_t width, uint32_t height, const std::string& file_path,
            Viewport viewport);
  virtual ~BmpCanvas() {}

  virtual void set_pixel(uint32_t x, uint32_t y, Rgba color) override;
  virtual Rgba get_pixel(uint32_t x, uint32_t y) const override;

  virtual void set_file_path(const std::string& new_path);
  virtual void display() override;
};

class WindowHandler;

class WindowCanvas : public Canvas {
 private:
  bool quit = false;

 protected:
  std::shared_ptr<WindowHandler> handler;
  virtual std::optional<Event> next_event() = 0;

 public:
  WindowCanvas() = delete;
  WindowCanvas(std::shared_ptr<WindowHandler>&& handler, Viewport viewport);
  virtual ~WindowCanvas() {}

  virtual void stop();
  virtual bool has_quit() const;
  virtual void update() override;
};

#ifdef USE_GLFW

class GLFWCanvas : public WindowCanvas {};

#endif

}  // namespace Canvas

#endif