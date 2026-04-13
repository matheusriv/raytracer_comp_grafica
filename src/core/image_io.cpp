#include "image_io.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../lib_stb_image_write/stb_image_write.h"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace ryt {

void write_ppm(const std::string& filename, const std::vector<RGBColor>& pixels, int w, int h, bool ascii) {
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Cannot open file " << filename << std::endl;
    return;
  }

  if (ascii) {
    file << "P3\n" << w << " " << h << "\n255\n";
    for (const auto& p : pixels) {
      int r = std::clamp(static_cast<int>(p.x * 255), 0, 255);
      int g = std::clamp(static_cast<int>(p.y * 255), 0, 255);
      int b = std::clamp(static_cast<int>(p.z * 255), 0, 255);
      file << r << " " << g << " " << b << "\n";
    }
  } else {
    file << "P6\n" << w << " " << h << "\n255\n";
    for (const auto& p : pixels) {
      unsigned char r = static_cast<unsigned char>(std::clamp(static_cast<int>(p.x * 255), 0, 255));
      unsigned char g = static_cast<unsigned char>(std::clamp(static_cast<int>(p.y * 255), 0, 255));
      unsigned char b = static_cast<unsigned char>(std::clamp(static_cast<int>(p.z * 255), 0, 255));
      file.write(reinterpret_cast<const char*>(&r), 1);
      file.write(reinterpret_cast<const char*>(&g), 1);
      file.write(reinterpret_cast<const char*>(&b), 1);
    }
  }
}

void write_png(const std::string& filename, const std::vector<RGBColor>& pixels, int w, int h) {
  std::vector<unsigned char> data(w * h * 3);
  for (size_t i = 0; i < pixels.size(); ++i) {
    data[i * 3 + 0] = static_cast<unsigned char>(std::clamp(static_cast<int>(pixels[i].x * 255), 0, 255));
    data[i * 3 + 1] = static_cast<unsigned char>(std::clamp(static_cast<int>(pixels[i].y * 255), 0, 255));
    data[i * 3 + 2] = static_cast<unsigned char>(std::clamp(static_cast<int>(pixels[i].z * 255), 0, 255));
  }

  stbi_flip_vertically_on_write(false);
  stbi_write_png(filename.c_str(), w, h, 3, data.data(), w * 3);
}

} // namespace ryt