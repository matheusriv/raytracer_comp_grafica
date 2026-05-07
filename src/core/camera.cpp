#include "camera.hpp"
#include <array>
#include <cmath>
#include <sstream>

namespace ryt {

// ==============================================================================
// Camera Base Class Implementation
// ==============================================================================

Camera::Camera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps)
    : m_film(std::move(film)) {

  // Retrieve lookat parameters
  auto look_from = lookat_ps.retrieve<Point3f>("look_from", Point3f{0, 0, -7});
  auto look_at = lookat_ps.retrieve<Point3f>("look_at", Point3f{0, 0, 0});
  auto up = lookat_ps.retrieve<Vector3f>("up", Vector3f{0, 1, 0});

  // Compute camera frame (orthonormal basis)
  // Note: 'gaze' is the direction the camera is looking at.
  Vector3f gaze = look_at - look_from;
  m_w = normalize(gaze);
  m_u = normalize(cross(m_w, up));  // left-hand rule
  m_v = normalize(cross(m_u, m_w));
  m_eye = look_from;

  // Determine the screen window bounds (left, right, bottom, top)
  auto screen_window_str = camera_ps.retrieve<std::string>("screen_window", "");
  if (!screen_window_str.empty()) {
    // If provided directly as a string, parse the 4 coordinates.
    std::istringstream iss(screen_window_str);
    iss >> m_l >> m_r >> m_b >> m_t;
  } else {
    // Otherwise, compute bounds from the vertical field of view (fovy) and aspect ratio.
    auto fovy = camera_ps.retrieve<real_type>("fovy", 30.0f);
    auto res = m_film->get_resolution();
    real_type aspect = static_cast<real_type>(res.x) / res.y;
    real_type tan_half_fovy = std::tan(fovy * M_PI / 180.0f / 2.0f);
    m_t = tan_half_fovy;
    m_b = -tan_half_fovy;
    m_r = aspect * tan_half_fovy;
    m_l = -m_r;
  }
}
Camera::~Camera() = default;

Film& Camera::film() const { return *m_film; }
Film* Camera::film_ptr() const { return m_film.get(); }

// ==============================================================================
// Perspective Camera Implementation
// ==============================================================================

PerspectiveCamera::PerspectiveCamera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps)
    : Camera(std::move(film), camera_ps, lookat_ps) {}

Rayf PerspectiveCamera::generate_ray(int x, int y) const {
  // Map pixel coordinates to Normalized Device Coordinates (NDC) then to screen space
  auto res = m_film->get_resolution();
  real_type nx = static_cast<real_type>(res.x);
  real_type ny = static_cast<real_type>(res.y);
  real_type u = m_l + (m_r - m_l) * (x + 0.5f) / nx;
  real_type v = m_t - (m_t - m_b) * (y + 0.5f) / ny;

  // Generate the perspective ray (origin at eye, direction through the screen plane)
  // Focal distance is implicitly assumed to be 1.0
  Vector3f direction = u * m_u + v * m_v + m_w;
  return Rayf{m_eye, direction};
}

// ==============================================================================
// Orthographic Camera Implementation
// ==============================================================================

OrthographicCamera::OrthographicCamera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps)
    : Camera(std::move(film), camera_ps, lookat_ps) {}

Rayf OrthographicCamera::generate_ray(int x, int y) const {
  // Map pixel coordinates to Normalized Device Coordinates (NDC) then to screen space
  auto res = m_film->get_resolution();
  real_type nx = static_cast<real_type>(res.x);
  real_type ny = static_cast<real_type>(res.y);
  real_type u = m_l + (m_r - m_l) * (x + 0.5f) / nx;
  real_type v = m_t - (m_t - m_b) * (y + 0.5f) / ny;

  // Generate the orthographic ray (origin shifted along the screen plane, fixed direction)
  Point3f origin = m_eye + u * m_u + v * m_v;
  return Rayf{origin, m_w};
}

// ==============================================================================
// Camera Factory Function
// ==============================================================================

Camera* create_camera(std::unique_ptr<Film> film, const ParamSet& camera_ps, const ParamSet& lookat_ps) {
  auto camera_type = camera_ps.retrieve<std::string>("type", "perspective");
  if (camera_type == "orthographic" || camera_type == "orthographic_camera" ||
      camera_type == "ortographic") {
    return new OrthographicCamera(std::move(film), camera_ps, lookat_ps);
  }
  return new PerspectiveCamera(std::move(film), camera_ps, lookat_ps);
}

}  // namespace ryt
