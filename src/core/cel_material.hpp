#ifndef CEL_MATERIAL_HPP
#define CEL_MATERIAL_HPP

#include <vector>
#include "material.hpp"
#include "geometry.hpp"

namespace ryt {

class CelMaterial : public Material {
public:
  std::vector<RGBColor> color_map;
  RGBColor shadow_color;
  RGBColor silhouette_color;
  real_type silhouette_angle_rad;

  CelMaterial(const std::vector<RGBColor>& cmap, const RGBColor& shadow, const RGBColor& silhouette, real_type sil_angle_deg)
      : color_map(cmap), shadow_color(shadow), silhouette_color(silhouette) {
        silhouette_angle_rad = sil_angle_deg * M_PI / 180.0f;
      }

  RGBColor color() const override {
    if (!color_map.empty()) return color_map.back(); // Returns the lightest color as base
    return color_black;
  }

  void scatter(Surfel& sf, const Rayf& r) const override {}
};

} // namespace ryt

#endif // CEL_MATERIAL_HPP