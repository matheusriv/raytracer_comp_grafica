#include <cmath>

#include "geometry.hpp"
#include "sphere.hpp"
#include "surfel.hpp"

namespace ryt {

Sphere::Sphere(const Point3f& center, real_type radius, bool flip_n)
    : Shape(flip_n), m_center{ center }, m_radius{ radius } {
}

Bounds3f Sphere::world_bounds() const {
  return Bounds3f(m_center - Vector3f(m_radius, m_radius, m_radius),
                  m_center + Vector3f(m_radius, m_radius, m_radius));
}

/// Solves the quadratic ray-sphere intersection equation.
/// Returns the smallest t in [r.t_min, r.t_max] that lies on the sphere, or -1 if none.
static real_type solve_quadratic(const Rayf& r, const Point3f& center, real_type radius) {
  Vector3f oc = r.o - center;
  real_type a = dot(r.d, r.d);

  // Algebraic reformulation to mitigate catastrophic cancellation (loss of precision)
  // using Lagrange's identity to express 1/4 of the discriminant
  Vector3f d_cross_oc = cross(r.d, oc);
  real_type discriminant_per_4 = a * (radius * radius) - dot(d_cross_oc, d_cross_oc);

  if (discriminant_per_4 < 0.0f) {
    return -1.0f;
  }

  real_type sqrt_disc = std::sqrt(discriminant_per_4);
  real_type half_b = dot(oc, r.d);

  real_type t1 = (-half_b - sqrt_disc) / a;
  real_type t2 = (-half_b + sqrt_disc) / a;

  if (t1 >= r.t_min && t1 <= r.t_max) return t1;
  if (t2 >= r.t_min && t2 <= r.t_max) return t2;
  return -1.0f;
}

bool Sphere::intersect(const Rayf& r, real_type* t_hit, Surfel* sf) const {
  real_type t = solve_quadratic(r, m_center, m_radius);
  if (t < 0.0f) return false;

  if (t_hit) *t_hit = t;

  if (sf != nullptr) {
    sf->p         = r(t);
    sf->n         = normalize(sf->p - m_center);
    if (flip_normals) sf->n = sf->n * -1.0f;
    sf->wo        = r.d * -1.0f;
    sf->time      = t;
    sf->uv        = Point2f{ 0.0f, 0.0f };
  }
  return true;
}

bool Sphere::intersect_p(const Rayf& r) const {
  return solve_quadratic(r, m_center, m_radius) >= 0.0f;
}

Sphere* create_sphere(const ParamSet& ps) {
  Point3f center = ps.retrieve<Point3f>("center", Point3f{ 0.0f, 0.0f, 0.0f });
  real_type radius = ps.retrieve<real_type>("radius", 1.0f);
  bool flip_n = ps.retrieve<bool>("flip_normals", false);
  return new Sphere(center, radius, flip_n);
}

}  // namespace ryt
