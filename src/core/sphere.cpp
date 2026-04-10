#include <cmath>

#include "geometry.hpp"
#include "sphere.hpp"

namespace ryt {

Sphere::Sphere(const Point3f& center, real_type radius, std::shared_ptr<Material> mat)
    : m_center{ center }, m_radius{ radius } {
  m_material = std::move(mat);
}

/// Solves the quadratic ray-sphere intersection equation.
/// Returns the smallest t in [r.t_min, r.t_max] that lies on the sphere, or -1 if none.
static real_type solve_quadratic(const Rayf& r, const Point3f& center, real_type radius) {
  Vector3f oc = r.o - center;
  real_type a = dot(r.d, r.d);
  real_type b = 2.0f * dot(oc, r.d);
  real_type c = dot(oc, oc) - radius * radius;
  real_type discriminant = b * b - 4.0f * a * c;

  if (discriminant < 0.0f) {
    return -1.0f;
  }

  real_type sqrt_disc = std::sqrt(discriminant);
  real_type t1 = (-b - sqrt_disc) / (2.0f * a);
  real_type t2 = (-b + sqrt_disc) / (2.0f * a);

  if (t1 >= r.t_min && t1 <= r.t_max) return t1;
  if (t2 >= r.t_min && t2 <= r.t_max) return t2;
  return -1.0f;
}

bool Sphere::intersect(const Rayf& r, Surfel* sf) const {
  real_type t = solve_quadratic(r, m_center, m_radius);
  if (t < 0.0f) return false;

  if (sf != nullptr) {
    sf->p         = r(t);
    sf->n         = normalize(sf->p - m_center);
    sf->wo        = r.d * -1.0f;
    sf->time      = t;
    sf->uv        = Point2f{ 0.0f, 0.0f };
    sf->primitive = this;
  }
  r.t_max = t;  // narrow the valid interval for subsequent hits (t_max is mutable)
  return true;
}

bool Sphere::intersect_p(const Rayf& r) const {
  return solve_quadratic(r, m_center, m_radius) >= 0.0f;
}

Sphere* create_sphere(const ParamSet& ps, std::shared_ptr<Material> mat) {
  Point3f center = ps.retrieve<Point3f>("center", Point3f{ 0.0f, 0.0f, 0.0f });
  real_type radius = ps.retrieve<real_type>("radius", 1.0f);
  return new Sphere(center, radius, std::move(mat));
}

}  // namespace ryt
