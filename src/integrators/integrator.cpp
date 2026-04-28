#include "integrator.hpp"
#include "raycast_integrator.hpp"
#include "normal_map_integrator.hpp"
#include "../core/surfel.hpp"
#include "../core/film.hpp"
#include <fstream>
#include <iostream>

namespace ryt {

void SamplerIntegrator::preprocess(const Scene& scene) {
  // Default empty implementation
}

void SamplerIntegrator::render(const Scene& scene) {
  preprocess(scene);

  auto width = camera->film().get_resolution().x;
  auto height = camera->film().get_resolution().y;

  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      Rayf ray = camera->generate_ray(w, h);

      auto L_opt = Li(ray, scene);
      RGBColor color;

      if (L_opt.has_value()) {
        color = L_opt.value();
      } else {
        // sample background color via screen-space UV.
        color = scene.background->sampleUV(float(w) / float(width), float(h) / float(height));
      }

      camera->film().add_sample(Point2i{w, h}, color);
    }
  }
  camera->film().write_image();
}

Integrator* create_integrator(std::shared_ptr<Camera> camera, const ParamSet& ps) {
  auto type = ps.retrieve<std::string>("type", "flat");
  if (type == "flat") {
    return new RayCastIntegrator(std::move(camera));
  }
  if (type == "normal_map") {
    return new NormalMapIntegrator(std::move(camera));
  }
  std::cerr << "Warning: Unknown integrator type '" << type << "', falling back to 'flat'.\n";
  return new RayCastIntegrator(std::move(camera));
}

} // namespace ryt