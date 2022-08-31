#include <fstream>

#include "canvas.h"

namespace Canvas {

BmpCanvas::BmpCanvas(uint32_t width, uint32_t height,
                     const std::string& file_path, Viewport viewport)
    : FrameBufferCanvas(width, height, viewport),
      file_path(file_path),
      pixels(width * height, NONE) {}

void BmpCanvas::set_pixel(uint32_t x, uint32_t y, Rgba color) {
  INDEX_SET(pixels, y * width + x, color);
}
Rgba BmpCanvas::get_pixel(uint32_t x, uint32_t y) const {
  Rgba color;
  INDEX_GET(color, pixels, y * width + x);
  return color;
}

void BmpCanvas::set_file_path(const std::string& new_path) {
  file_path = new_path;
}

void BmpCanvas::display() {
  uint32_t padding = (4 - ((width * 3) % 4)) % 4;
  uint32_t header_size = 14;
  uint32_t info_header_size = 40;
  uint32_t file_size =
      header_size + info_header_size + height * padding + height * width * 3;
  uint32_t zero = 0;
  uint32_t data_offset = header_size + info_header_size;
  uint16_t planes = 1;
  uint16_t bits_per_pixel = 24;
  uint32_t used_colors = 16777216;

  auto f = std::ofstream(file_path, std::ios::binary);
  if (!f.is_open()) {
    std::cerr << "Could not open file " << file_path << "\n";
    exit(1);
  }

  f.write("BM", 2);
  f.write((char*)&file_size, 4);
  f.write((char*)&zero, 4);
  f.write((char*)&data_offset, 4);

  f.write((char*)&info_header_size, 4);
  f.write((char*)&width, 4);
  f.write((char*)&height, 4);
  f.write((char*)&planes, 2);
  f.write((char*)&bits_per_pixel, 2);
  f.write((char*)&zero, 4);
  f.write((char*)&zero, 4);
  f.write((char*)&width, 4);
  f.write((char*)&height, 4);
  f.write((char*)&used_colors, 4);
  f.write((char*)&zero, 4);

  for (uint32_t y = 0; y < height; y++) {
    // uint32_t y = height - i - 1;
    for (uint32_t x = 0; x < width; x++) {
      size_t index = (y * width + x);
      uint8_t r = pixels[index].b * 255.0;
      uint8_t g = pixels[index].g * 255.0;
      uint8_t b = pixels[index].r * 255.0;
      f.write((char*)&r, 1);
      f.write((char*)&g, 1);
      f.write((char*)&b, 1);
    }

    for (uint32_t i = 0; i < padding; i++) {
      f.write((char*)&zero, 1);
    }
  }

  if (!f) {
    std::cerr << "Error writing to file " << file_path << "\n";
    exit(1);
  }

  f.close();
}

}  // namespace Canvas
