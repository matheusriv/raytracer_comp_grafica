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

SpotLight::SpotLight(const Point3f& p, const Point3f& to, const RGBColor& I, float cutoff_deg, float falloff_deg)
    : Light(light_flag_e::spot), pLight(p), dir(normalize(to - p)), I(I) {
  // Convert angles from degrees to radians.
  cutoff_rad = cutoff_deg * M_PI / 180.0f;
  falloff_rad = falloff_deg * M_PI / 180.0f;
  
  // Precompute cosines for faster comparison during ray tracing.
  cos_cutoff = std::cos(cutoff_rad);
  cos_falloff = std::cos(falloff_rad);
}

RGBColor SpotLight::sample_Li(const Surfel& hit, Vector3f* wi, VisibilityTester* vis) {
  // Calculate the normalized direction vector from the hit point towards the light source.
  *wi = normalize(pLight - hit.p);
  if (vis) { vis->p0 = hit.p + normalize(hit.n) * 0.05f; vis->p1 = pLight; }

  // Calculate the cosine of the angle between the spotlight's main direction 
  // and the vector pointing from the light source to the hit point.
  float cos_theta = dot(*wi * -1.0f, dir);
  
  // If the angle is greater than the cutoff angle (i.e., its cosine is smaller),
  // the point is completely outside the spotlight's cone of influence.
  if (cos_theta < cos_cutoff) {
    return color_black;
  }

  // Default weight is 1.0 (full intensity), applied if the point is inside the inner falloff cone.
  float weight = 1.0f;
  
  // If the point is in the transition zone (penumbra) between the falloff and cutoff cones:
  if (cos_theta < cos_falloff && std::abs(cos_falloff - cos_cutoff) > 1e-5f) {
    // Calculate a linear interpolation factor delta (0.0 at the cutoff edge, 1.0 at the falloff edge).
    float delta = (cos_theta - cos_cutoff) / (cos_falloff - cos_cutoff);
    // Apply a cubic curve (delta^3) for a smoother and faster intensity decay towards the edge.
    weight = delta * delta * delta;
  }

  return I * weight;
}

} // namespace ryt