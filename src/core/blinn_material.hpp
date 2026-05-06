#ifndef BLINN_MATERIAL_HPP
#define BLINN_MATERIAL_HPP

#include "material.hpp"
#include "geometry.hpp"

namespace ryt {

class BlinnPhongMaterial : public Material {
public:
  RGBColor ka;
  RGBColor kd;
  RGBColor ks;
  float glossiness;

  BlinnPhongMaterial(const RGBColor& ka, const RGBColor& kd, const RGBColor& ks, float g)
      : ka(ka), kd(kd), ks(ks), glossiness(g) {}

  RGBColor color() const override { return kd; }

  void scatter(Surfel& sf, const Rayf& r) const override {}
};

} // namespace ryt

#endif // BLINN_MATERIAL_HPP