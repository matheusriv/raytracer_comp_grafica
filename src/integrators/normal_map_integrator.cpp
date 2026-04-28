#include "normal_map_integrator.hpp"
#include "../core/surfel.hpp"
#include "../core/geometry.hpp"

namespace ryt {

std::optional<RGBColor> NormalMapIntegrator::Li(const Rayf& ray, const Scene& scene) const {
  Surfel sf;
  if (!scene.intersect(ray, &sf)) {
    return std::nullopt; // Missed the scene geometry, fallback to background.
  }

  Vector3f n = normalize(sf.n);

  RGBColor color {
    (n.x + 1.0f) * 0.5f,
    (n.y + 1.0f) * 0.5f,
    (n.z + 1.0f) * 0.5f
  };

  return color;
}

} // namespace ryt