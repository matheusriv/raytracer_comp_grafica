#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include "scene.hpp"
#include "film.hpp"
#include "camera.hpp"
#include "common.hpp"
#include <fstream>

namespace ryt {

class Integrator {
public:
  void render(const Camera& camera, const Scene& scene) {
    std::ofstream ray_file("../results/ray_debug.txt");
    if (!ray_file.is_open()) {
      std::cerr << "Failed to open ray_debug.txt\n";
      return;
    }
    auto width = camera.film().get_resolution().x;
    auto height = camera.film().get_resolution().y;
    for(int h = height-1 ; h >= 0 ; h--) {
      for(int w = 0 ; w < width ; w++) {
        Rayf ray = camera.generate_ray(w, h);
        ray_file << "@ pixel(" << w << "," << h << "), " << ray << std::endl;
        auto color = scene.background->sampleUV(float(w) / float(width), float(h) / float(height));
        camera.film().add_sample(Point2i{w, h}, color);
      }
    }
    camera.film().write_image();
  }
  virtual ~Integrator() = default;
};

} // namespace ryt

#endif