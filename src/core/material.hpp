#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "common.hpp"
#include "geometry.hpp"

namespace ryt {

/// Abstract base class for all materials.
class Material {
public:
  virtual ~Material() = default;
  virtual RGBColor color() const = 0;
};

/// Flat (unlit) material that returns a constant color regardless of lighting.
class FlatMaterial : public Material {
public:
  explicit FlatMaterial(const RGBColor& c) : m_color{ c } {}
  RGBColor color() const override { return m_color; }

private:
  RGBColor m_color;
};

}  // namespace ryt

#endif  // MATERIAL_HPP
