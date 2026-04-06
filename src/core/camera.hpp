#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <memory>

#include "film.hpp"
#include "geometry.hpp"
#include "paramset.hpp"

namespace ryt {

class Camera {
public:
  explicit Camera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps);
  virtual ~Camera();

  [[nodiscard]] Film& film() const;
  [[nodiscard]] Film* film_ptr() const;

  virtual Rayf generate_ray(int x, int y) const = 0;

  const Point3f& eye() const { return m_eye; }
  const Vector3f& u() const { return m_u; }
  const Vector3f& v() const { return m_v; }
  const Vector3f& w() const { return m_w; }

  real_type left() const { return m_l; }
  real_type right() const { return m_r; }
  real_type bottom() const { return m_b; }
  real_type top() const { return m_t; }

protected:
  std::unique_ptr<Film> m_film;
  // Camera frame
  Point3f m_eye;      // look_from
  Vector3f m_u, m_v, m_w;  // orthonormal basis
  // Screen window
  real_type m_l, m_r, m_b, m_t;  // left, right, bottom, top
};

class PerspectiveCamera : public Camera {
public:
  explicit PerspectiveCamera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps);
  Rayf generate_ray(int x, int y) const override;
};

class OrthographicCamera : public Camera {
public:
  explicit OrthographicCamera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps);
  Rayf generate_ray(int x, int y) const override;
};

Camera* create_camera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps);

}  // namespace ryt

#endif  // CAMERA_HPP
