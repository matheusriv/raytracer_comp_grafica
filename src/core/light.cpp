#include "light.hpp"
#include "scene.hpp"
#include <cmath>

namespace ryt {

bool VisibilityTester::unoccluded(const Scene& scene) const {
  Vector3f dir = p1 - p0;
  float dist = std::sqrt(dot(dir, dir));
  dir = normalize(dir);
  Rayf ray(p0, dir); // The offset to avoid shadow acne is now handled in sample_Li using the normal
  Surfel sf;
  if (scene.intersect(ray, &sf)) {
    Vector3f hit_dir = sf.p - p0;
    float hit_dist = std::sqrt(dot(hit_dir, hit_dir));
    if (hit_dist < dist - 0.05f) { // If we hit something before reaching the light, it's occluded
      return false;
    }
  }
  return true;
}

RGBColor PointLight::sample_Li(const Surfel& hit, Vector3f* wi, VisibilityTester* vis) {
  *wi = normalize(pLight - hit.p);
  // Epsilon offset from "hit.p" along the Normal prevents circular artifacts
  if (vis) { vis->p0 = hit.p + normalize(hit.n) * 0.05f; vis->p1 = pLight; }
  return I;
}

RGBColor DirectionalLight::sample_Li(const Surfel& hit, Vector3f* wi, VisibilityTester* vis) {
  *wi = dir;
  // Epsilon offset from "hit.p" along the Normal prevents circular artifacts
  if (vis) { vis->p0 = hit.p + normalize(hit.n) * 0.05f; vis->p1 = hit.p + dir * 10000.0f; }
  return I;
}

RGBColor AmbientLight::sample_Li(const Surfel& hit, Vector3f* wi, VisibilityTester* vis) {
  *wi = Vector3f{0.0f, 1.0f, 0.0f}; // Arbitrary orientation for ambient
  return I;
}

} // namespace ryt