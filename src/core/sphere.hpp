#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "shape.hpp"
#include "paramset.hpp"

namespace ryt {

/// Sphere primitive defined by a center point and a radius.
class Sphere : public Shape {
public:
  Sphere(const Point3f& center, real_type radius, bool flip_n = false);

  Bounds3f world_bounds() const override;
  bool intersect(const Rayf& r, real_type* t_hit, Surfel* sf) const override;
  bool intersect_p(const Rayf& r) const override;

private:
  Point3f m_center;
  real_type m_radius;
};

Sphere* create_sphere(const ParamSet& ps);

}  // namespace ryt

#endif  // SPHERE_HPP
