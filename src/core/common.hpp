#ifndef COMMON_HPP
#define COMMON_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

namespace ryt {

// ==============================================================================
// Aliases & Types
// ==============================================================================

/// Forward declaration for Vector3 type
template <typename T>
class Vector3;

using real_type = float;
using size_type = size_t;
using byte = std::uint8_t;
using RGBColor = Vector3<real_type>;

// ==============================================================================
// Runtime Options
// ==============================================================================

/// Structure containing runtime options parsed from CLI arguments
struct RunningOptions {
  RunningOptions(std::string filename = "", std::string outfile = "")
      : filename(std::move(filename)), outfile(std::move(outfile)) {}

  /*!
   * Crop window to render.
   * [ x0, x1 ] -> X values
   * [ y0, y1 ] -> Y values
   * where x0,x1,y0,y1 in [0.0, 1.0].
   * 1 = 100% of the full resolution.
   */
  std::array<float, 4> crop_window{ 0, 1, 0, 1 };
  std::string filename;               //!< Input scene file name.
  std::string outfile;                //!< Output image file name.
  bool quick_render{ false };         //!< When set, render image with 1/4 of the requested resolution.
  bool verbose{ false };              //!< When set, the program shows lots of debug messages.
  bool crop_window_provided{ false }; //!< When set, we got crop window specification via CLI.
};

// ==============================================================================
// String Utilities
// ==============================================================================

/// Lambda expression that returns a lowercase version of the input string.
inline auto str_lowercase = [](std::string str) -> std::string {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
};

/// Lambda expression that transforms a C-style string to a lowercase C++ string version.
static auto str_to_lower = [](const char* c_str) -> std::string {
  std::string str{ c_str };
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
};

} // namespace ryt

#endif  // COMMON_HPP
