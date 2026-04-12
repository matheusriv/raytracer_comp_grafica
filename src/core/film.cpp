#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../../msg_system/error.hpp"
#include "App.hpp"
#include "common.hpp"
#include "film.hpp"
#include "geometry.hpp"
#include "image_io.hpp"
#include "paramset.hpp"
#include <string_view>

namespace ryt {

//=== Film Method Definitions
Film::Film(const Point2i& resolution,
           const std::string& filename,
           image_type_e image_type,
           bool gamma_corrected)
    : m_full_resolution{ resolution }, m_filename{ filename },
      m_activate_gamma_correction{ gamma_corrected }, m_image_type{ image_type } {
  m_pixels.resize(m_full_resolution.x * m_full_resolution.y);
}

Film::~Film() = default;

/// Add the RGBColor to image. Pixel coords comes as (x,y).
void Film::add_sample(const Point2i& pixel_coord, const RGBColor& pixel_color) {
  int x = pixel_coord.x;
  int y = pixel_coord.y;
  if (x >= 0 && x < m_full_resolution.x && y >= 0 && y < m_full_resolution.y) {
    size_t index = static_cast<size_t>(y) * m_full_resolution.x + x;
    m_pixels[index] = pixel_color;
  }
}

/// Convert Spectrum image information to RGB, compute final pixel values, write image.
void Film::write_image() const {
  if (m_image_type == image_type_e::PPM3 || m_image_type == image_type_e::PPM6) {
    bool ascii = (m_image_type == image_type_e::PPM3);
    write_ppm(m_filename, m_pixels, m_full_resolution.x, m_full_resolution.y, ascii);
  } else if (m_image_type == image_type_e::PNG) {
    write_png(m_filename, m_pixels, m_full_resolution.x, m_full_resolution.y);
  } else {
    std::cerr << "Image format not supported." << std::endl;
  }
}

/// Chooses the filename based on the CLI and scene file info.
std::string handles_filename(const ParamSet& ps) {
  std::string filename;
  // Se o usuário forneceu um arquivo de saída via CLI, ele tem prioridade.
  if (!App::m_current_run_options.outfile.empty()) {
    filename = App::m_current_run_options.outfile;
  } else {
    // Caso contrário, recupera do arquivo de cena (XML)
    filename = ps.retrieve<std::string>("filename", "output");
  }

  std::string img_type = ps.retrieve<std::string>("img_type", "png");

  // Se o nome do arquivo já possuir extensão, removemos para pegar apenas o nome
  size_t last_dot = filename.find_last_of('.');
  size_t last_slash = filename.find_last_of("/\\");
  if (last_dot != std::string::npos && (last_slash == std::string::npos || last_dot > last_slash)) {
    filename = filename.substr(0, last_dot);
  }
  
  // Adicionamos a extensão apropriada com base no tipo de imagem solicitado
  filename += (img_type == "png") ? ".png" : ".ppm";

  // Se for apenas um nome de arquivo simples (sem caminhos de diretório indicados), 
  // salvamos por padrão no diretório results/
  if (filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos) {
    filename = "../results/" + filename;
  }

  return filename;
}

// /// Process ParamSet, extracts, validates a valid crop window.
// ryt::Bounds2f handles_cropwindow(const ParamSet& ps) {
//   
// }

/// Parses the dimensions of the film from the ParamSet.
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

/// Creates and returns a `Film` objected based on the `ParamSet` provided.
Film* create_film(const ParamSet& ps) {
#ifdef DEBUG
  std::cout << ">>> Inside create_film()\n";
#endif
  //==[1] Choose the filename.
  auto filename = handles_filename(ps);

  //==[2] Define the crop window information.
  // auto crop_window = handles_cropwindow(ps);

  //==[3] Retrieve film dimensions and handles quick_render option.
  Point2i dimensions = handles_dimensions(ps);

  //==[4] Retrieve film type.
  std::unordered_map<std::string, Film::image_type_e> image_type{
    { "png", Film::image_type_e::PNG },
    { "ppm3", Film::image_type_e::PPM3 },
    { "ppm6", Film::image_type_e::PPM6 },
    { "ppm", Film::image_type_e::PPM6 },
  };
  auto type{ image_type[ps.retrieve<std::string>("img_type", "png")] };

  //==[5] Get gamma correction request.
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
}  // namespace
