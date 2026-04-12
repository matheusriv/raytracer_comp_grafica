#ifndef COMMON_HPP
#define COMMON_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

namespace ryt {

//=== aliases
// Forward declarations
template <typename T>
class Vector3;

using real_type = float;
using size_type = size_t;
using byte = std::uint8_t;
using RGBColor = Vector3<real_type>;

// Add here Option structure.
struct RunningOptions {
  RunningOptions(std::string filename = "", std::string outfile = "")
      : filename(std::move(filename)), outfile(std::move(outfile)) {}
  // -----------------------------------------
  // [ x0, x1 ] -> X values
  // [ y0, y1 ] -> Y values
  // where x0,x1,y0,y1 in [0.0, 1.0].
  // 1 = 100% of the full resolution.
  // -----------------------------------------
  /// Crop window to render.
  std::array<float, 4> crop_window{ 0, 1, 0, 1 };
  std::string filename;        //!< input scene file name.
  std::string outfile;         //!< output image file name.
  bool quick_render{ false };  //!< when set, render image with 1/4 of the requested resolition.
  bool verbose{ false };       //!< when set, the program shows lots of debug message.
  bool crop_window_provided{ false };  //!< when set, we got crop window specification via CLI.
};

/// Lambda expression that returns a lowercase version of the input string.
inline auto str_lowercase = [](std::string str) -> std::string {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
};

// Lambda expression that transform a c-style string to a lowercase c++-stype string version.
static auto str_to_lower = [](const char* c_str) -> std::string {
  std::string str{ c_str };
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
};

} // namespace

#endif  // !COMMON_HPP
