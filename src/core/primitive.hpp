#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include <memory>

#include "geometry.hpp"
#include "material.hpp"
#include "surfel.hpp"

namespace ryt {

/// Abstract base class for any surface that a ray might intersect.
class Primitive {
public:
  virtual ~Primitive() = default;

  /// Computes the intersection of the ray with this primitive.
  /// If an intersection is found within [r.t_min, r.t_max], updates r.t_max to the hit t and
  /// fills in the Surfel *sf (if not nullptr). Returns true on hit.
  virtual bool intersect(const Rayf& r, Surfel* sf) const = 0;

  /// Faster shadow-ray version: only returns true/false, does not fill Surfel.
  virtual bool intersect_p(const Rayf& r) const = 0;

  const Material* get_material() const { return m_material.get(); }

protected:
  std::shared_ptr<Material> m_material;
};

}  // namespace ryt

#endif  // PRIMITIVE_HPP
