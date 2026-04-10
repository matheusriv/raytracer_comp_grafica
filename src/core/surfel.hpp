#ifndef SURFEL_HPP
#define SURFEL_HPP

#include "geometry.hpp"

namespace ryt {

class Primitive;

/// Represents the geometry of a particular surface contact point found by intersecting a ray.
struct Surfel {
  Surfel() = default;
  Surfel(const Point3f& p, const Vector3f& n, const Vector3f& wo, real_type time,
         const Point2f& uv, const Primitive* prim)
      : p{ p }, n{ n }, wo{ wo }, time{ time }, uv{ uv }, primitive{ prim } {}

  Point3f p;                       //!< Contact point.
  Vector3f n;                      //!< The surface normal at contact point.
  Vector3f wo;                     //!< Outgoing light direction (-ray.d).
  real_type time{ 0 };             //!< Ray parameter t at the hit point.
  Point2f uv;                      //!< Parametric (u,v) coordinate on the surface.
  const Primitive* primitive{ nullptr };  //!< Pointer to the hit primitive.
};

}  // namespace ryt

#endif  // SURFEL_HPP
