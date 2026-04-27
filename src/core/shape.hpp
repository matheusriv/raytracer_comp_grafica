#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "geometry.hpp"

namespace ryt {

struct Surfel;

class Shape {
public:
  Shape(bool flip_n = false) : flip_normals{flip_n} {}
  virtual ~Shape() = default;

  virtual Bounds3f world_bounds() const = 0;
  virtual bool intersect(const Rayf& r, real_type* t_hit, Surfel* sf) const = 0;
  virtual bool intersect_p(const Rayf& r) const = 0;

  bool flip_normals;
};

}  // namespace ryt

#endif  // SHAPE_HPP