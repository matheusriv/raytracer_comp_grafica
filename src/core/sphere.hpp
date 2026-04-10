#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "primitive.hpp"
#include "paramset.hpp"

namespace ryt {

/// Sphere primitive defined by a center point and a radius.
class Sphere : public Primitive {
public:
  Sphere(const Point3f& center, real_type radius, std::shared_ptr<Material> mat);

  bool intersect(const Rayf& r, Surfel* sf) const override;
  bool intersect_p(const Rayf& r) const override;

private:
  Point3f m_center;
  real_type m_radius;
};

Sphere* create_sphere(const ParamSet& ps, std::shared_ptr<Material> mat);

}  // namespace ryt

#endif  // SPHERE_HPP
