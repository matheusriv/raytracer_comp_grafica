#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "geometry.hpp"
#include "transform.hpp"

namespace ryt {

struct Surfel;

class Shape {
public:
  Shape(const Transform* o2w = nullptr, const Transform* w2o = nullptr, bool flip_n = false)
      : obj_to_world(o2w), world_to_obj(w2o), flip_normals(flip_n) {}
  virtual ~Shape() = default;

  virtual Bounds3f world_bounds() const = 0;
  virtual bool intersect(const Rayf& r, real_type* t_hit, Surfel* sf) const = 0;
  virtual bool intersect_p(const Rayf& r) const = 0;

  const Transform* obj_to_world;
  const Transform* world_to_obj;
  bool flip_normals;
};

}  // namespace ryt

#endif  // SHAPE_HPP