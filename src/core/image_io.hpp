#ifndef IMAGE_IO_HPP
#define IMAGE_IO_HPP

#include <string>
#include <vector>

#include "common.hpp"
#include "geometry.hpp"

namespace ryt {

void write_ppm(const std::string& filename, const std::vector<RGBColor>& pixels, int w, int h, bool ascii = false);
void write_png(const std::string& filename, const std::vector<RGBColor>& pixels, int w, int h);

} // namespace ryt

#endif