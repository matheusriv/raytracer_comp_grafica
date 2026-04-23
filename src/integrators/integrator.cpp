#include "integrator.hpp"
#include "../core/surfel.hpp"
#include "../core/film.hpp"
#include <fstream>
#include <iostream>

namespace ryt {

void FlatIntegrator::render(const Scene& scene) {
  std::ofstream ray_file("../results/ray_debug.txt");
  if (!ray_file.is_open()) {
    std::cerr << "Failed to open ray_debug.txt\n";
    return;
  }
  auto width = camera->film().get_resolution().x;
  auto height = camera->film().get_resolution().y;
  for(int h = 0 ; h < height ; h++) {
    for(int w = 0 ; w < width ; w++) {
      Rayf ray = camera->generate_ray(w, h);
      ray_file << "@ pixel(" << w << "," << h << "), " << ray << std::endl;

      // sample background color via screen-space UV.
      auto color = scene.background->sampleUV(
          float(w) / float(width), float(h) / float(height));

      // Test each primitive; intersect() narrows ray.t_max so the first hit wins.
      Surfel sf;
      for (const auto& prim : scene.primitives) {
        if (prim->intersect(ray, &sf)) {
          const Material* mat = prim->get_material();
          if (mat != nullptr) {
            color = mat->color();
          }
        }
      }

      camera->film().add_sample(Point2i{w, h}, color);
    }
  }
  camera->film().write_image();
}

Integrator* create_integrator(std::shared_ptr<Camera> camera, const ParamSet& ps) {
  auto type = ps.retrieve<std::string>("type", "flat");
  if (type == "flat") {
    return new FlatIntegrator(std::move(camera));
  }
  std::cerr << "Warning: Unknown integrator type '" << type << "', falling back to 'flat'.\n";
  return new FlatIntegrator(std::move(camera));
}

} // namespace ryt