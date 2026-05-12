#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "../../msg_system/error.hpp"
#include "App.hpp"
#include "common.hpp"
#include "film.hpp"
#include "geometry.hpp"
#include "image_io.hpp"
#include "paramset.hpp"
#include <string_view>
#include <filesystem>

namespace fs = std::filesystem;

namespace ryt {

// ==============================================================================
// Film Class Implementation
// ==============================================================================

Film::Film(const Point2i& resolution,
           const std::string& filename,
           image_type_e image_type,
           bool gamma_corrected)
    : m_full_resolution{ resolution }, m_filename{ filename },
      m_activate_gamma_correction{ gamma_corrected }, m_image_type{ image_type } {
  m_pixels.resize(m_full_resolution.x * m_full_resolution.y);
}

Film::~Film() = default;

/*!
 * Adds a computed sample (pixel color) to the image at the given pixel coordinates.
 * Checks boundaries to prevent out-of-bounds memory access.
 */
void Film::add_sample(const Point2i& pixel_coord, const RGBColor& pixel_color) {
  int x = pixel_coord.x;
  int y = pixel_coord.y;
  if (x >= 0 && x < m_full_resolution.x && y >= 0 && y < m_full_resolution.y) {
    size_t index = static_cast<size_t>(y) * m_full_resolution.x + x;
    m_pixels[index] = pixel_color;
  }
}

/// Writes the accumulated pixel data to a file using the configured image format (PPM or PNG).
void Film::write_image() const {
  std::vector<RGBColor> output_pixels = m_pixels;
  
  // Apply gamma correction to the pixels before saving
  if (m_activate_gamma_correction) {
    float inv_gamma = 1.0f / 2.2f;
    for (auto& color : output_pixels) {
      color.x = std::pow(color.x, inv_gamma);
      color.y = std::pow(color.y, inv_gamma);
      color.z = std::pow(color.z, inv_gamma);
    }
  }

  bool saved = false;
  if (m_image_type == image_type_e::PPM3 || m_image_type == image_type_e::PPM6) {
    bool ascii = (m_image_type == image_type_e::PPM3);
    write_ppm(m_filename, output_pixels, m_full_resolution.x, m_full_resolution.y, ascii);
    saved = true;
  } else if (m_image_type == image_type_e::PNG) {
    write_png(m_filename, output_pixels, m_full_resolution.x, m_full_resolution.y);
    saved = true;
  } else {
    std::cerr << "Image format not supported." << std::endl;
  }

  if (saved) {
    std::cout << "\n" << m_filename << " saved!\n" << std::endl;
  }
}

// ==============================================================================
// Standalone / Helper Functions
// ==============================================================================

/// Chooses and constructs the correct output filename based on CLI arguments and the scene file parameters.
std::string handles_filename(const ParamSet& ps) {
  std::string filename;
  bool from_cli = false;

  // If the user provided an output file via CLI, it takes priority.
  if (!App::m_current_run_options.outfile.empty()) {
    filename = App::m_current_run_options.outfile;
    from_cli = true;
  } else {
    // Otherwise, retrieve from the scene file (XML)
    filename = ps.retrieve<std::string>("filename", "output");
  }

  std::string img_type = ps.retrieve<std::string>("img_type", "png");

  // If the filename already has an extension, remove it to get only the name
  size_t last_dot = filename.find_last_of('.');
  size_t last_slash = filename.find_last_of("/\\");
  if (last_dot != std::string::npos && (last_slash == std::string::npos || last_dot > last_slash)) {
    filename = filename.substr(0, last_dot);
  }
  
  // Add the appropriate extension based on the requested image type
  filename += (img_type == "png") ? ".png" : ".ppm";

  // If it's just a simple filename (without any directory paths), 
  // save it by default in the results/ directory
  if (filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos) {
    filename = "../results/" + filename;
  } else if (!from_cli) {
    // If it contains a slash (e.g. "./output") and didn't come from the CLI, we resolve the relative path
    // starting from the directory of the original XML scene file itself.
    fs::path path_obj(filename);
    if (path_obj.is_relative()) {
      fs::path xml_dir = fs::path(App::m_current_run_options.filename).parent_path();
      if (!xml_dir.empty()) {
        filename = (xml_dir / path_obj).lexically_normal().generic_string();
      }
    }
  }

  return filename;
}

// Processes ParamSet, extracts, and validates a valid crop window.
// ryt::Bounds2f handles_cropwindow(const ParamSet& ps) {
//   
// }

/// Parses the dimensions of the film from the ParamSet and applies quick-render downscaling if requested.
ryt::Point2i handles_dimensions(const ParamSet& ps) {
  int width = ps.retrieve<int>("w_res", ps.retrieve<int>("x_res", 1280));
  int height = ps.retrieve<int>("h_res", ps.retrieve<int>("y_res", 720));
  Point2i film_dimension{ width, height };
  if (App::m_current_run_options.quick_render) {
    // decrease resolution.
    film_dimension.x = std::max(1, film_dimension.x / 4);
    film_dimension.y = std::max(1, film_dimension.y / 4);
  }
  //std::cout << ">>> Film resolution set to: " << film_dimension.x << "x" << film_dimension.y << "\n";
  return film_dimension;
}

// ==============================================================================
// Factory Function
// ==============================================================================

/// Factory function that creates and returns a Film object based on the provided ParamSet.
Film* create_film(const ParamSet& ps) {
#ifdef DEBUG
  std::cout << ">>> Inside create_film()\n";
#endif
  // Choose the filename.
  auto filename = handles_filename(ps);

  // Define the crop window information.
  // auto crop_window = handles_cropwindow(ps);

  // Retrieve film dimensions and handles quick_render option.
  Point2i dimensions = handles_dimensions(ps);

  // Retrieve film type.
  std::unordered_map<std::string, Film::image_type_e> image_type{
    { "png", Film::image_type_e::PNG },
    { "ppm3", Film::image_type_e::PPM3 },
    { "ppm6", Film::image_type_e::PPM6 },
    { "ppm", Film::image_type_e::PPM6 },
  };
  auto type{ image_type[ps.retrieve<std::string>("img_type", "png")] };

  // Get gamma correction request.
  bool apply_gamma_correction = ps.retrieve<bool>("gamma_corrected", false);

#ifdef DEBUG
  std::cout << "================================================\n";
  std::cout << ">>> create_film() - film parameters are:\n";
  std::cout << "    - filename: " << std::quoted(filename) << "\n";
  // std::cout << "    - crop window: " << crop_window << "\n";
  std::cout << "    - w_res: " << dimensions.x << "\n";
  std::cout << "    - h_res: " << dimensions.y << "\n";
  std::cout << "    - image type: " << ps.retrieve<std::string>("img_type", "png") << "\n";
  std::cout << "    - gamma correction: " << std::boolalpha << apply_gamma_correction << "\n";
  std::cout << "================================================\n";
#endif

  return new Film(dimensions, filename, type, apply_gamma_correction);
}

}  // namespace ryt
