#include "raycast_integrator.hpp"
#include "../core/surfel.hpp"

namespace ryt {

std::optional<RGBColor> RayCastIntegrator::Li(const Rayf& ray, const Scene& scene) const {
  Surfel sf;
  if (!scene.intersect(ray, &sf)) {
    return std::nullopt; // Missed the scene geometry, fallback to background.
  }
  const Material* mat = sf.primitive->get_material();
  if (mat != nullptr) {
    return mat->color();
  }
  return color_black;
}

} // namespace ryt