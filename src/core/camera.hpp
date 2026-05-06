#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <memory>

#include "film.hpp"
#include "geometry.hpp"
#include "paramset.hpp"

namespace ryt {

/// Abstract base class for all camera models.
/*!
 * The camera is responsible for generating rays for each pixel of the film.
 * It also holds the film object where the image is rendered and maintains 
 * the viewing coordinate frame and the screen window parameters.
 */
class Camera {
public:
  /// Constructs a base camera with a film, camera parameters, and lookat parameters.
  explicit Camera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps);
  virtual ~Camera();

  /// Returns a reference to the film object.
  [[nodiscard]] Film& film() const;
  /// Returns a pointer to the film object.
  [[nodiscard]] Film* film_ptr() const;

  /// Generates a ray for a given pixel coordinate (x, y) on the screen.
  virtual Rayf generate_ray(int x, int y) const = 0;

  // ==============================================================================
  // Accessors for Camera Frame and Screen Window
  // ==============================================================================

  const Point3f& eye() const { return m_eye; }
  const Vector3f& u() const { return m_u; }
  const Vector3f& v() const { return m_v; }
  const Vector3f& w() const { return m_w; }

  real_type left() const { return m_l; }
  real_type right() const { return m_r; }
  real_type bottom() const { return m_b; }
  real_type top() const { return m_t; }

protected:
  std::unique_ptr<Film> m_film;   //!< The film where the generated image will be stored.

  // ==============================================================================
  // Camera Frame
  // ==============================================================================
  
  Point3f m_eye;                  //!< Camera origin (look_from point).
  Vector3f m_u, m_v, m_w;         //!< Orthonormal basis for the camera's local coordinate system.

  // ==============================================================================
  // Screen Window
  // ==============================================================================
  
  real_type m_l;                  //!< Left bound of the screen window.
  real_type m_r;                  //!< Right bound of the screen window.
  real_type m_b;                  //!< Bottom bound of the screen window.
  real_type m_t;                  //!< Top bound of the screen window.
};

/// Implements a camera using a perspective projection.
class PerspectiveCamera : public Camera {
public:
  explicit PerspectiveCamera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps);
  
  /// Generates a perspective viewing ray.
  Rayf generate_ray(int x, int y) const override;
};

/// Implements a camera using an orthographic projection.
class OrthographicCamera : public Camera {
public:
  explicit OrthographicCamera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps);
  
  /// Generates an orthographic viewing ray.
  Rayf generate_ray(int x, int y) const override;
};

/// Factory function to instantiate the proper camera type based on parsed attributes.
Camera* create_camera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps);

}  // namespace ryt

#endif  // CAMERA_HPP
