#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <cstdint>
#include <memory>
#include "geometry.hpp"
#include "surfel.hpp"

namespace ryt {

class Scene;

enum class light_flag_e : std::uint8_t {
  point = 1,
  directional = 2,
  area = 4,
  ambient = 8,
  spot = 16
};

inline bool is_ambient(light_flag_e flag) {
  return static_cast<std::uint8_t>(flag) & static_cast<std::uint8_t>(light_flag_e::ambient);
}

inline bool is_directional(light_flag_e flag) {
  return static_cast<std::uint8_t>(flag) & static_cast<std::uint8_t>(light_flag_e::directional);
}

inline bool is_point(light_flag_e flag) {
  return static_cast<std::uint8_t>(flag) & static_cast<std::uint8_t>(light_flag_e::point);
}

struct VisibilityTester {
  Point3f p0;
  Point3f p1;
  
  bool unoccluded(const Scene& scene) const;
};

class Light {
public:
  light_flag_e flags;

  virtual ~Light() = default;
  Light(light_flag_e flags) : flags(flags) {}

  virtual RGBColor sample_Li(const Surfel& hit, Vector3f* wi, VisibilityTester* vis) = 0;
  virtual void preprocess(const Scene&) {}
};

class PointLight : public Light {
public:
  Point3f pLight;
  RGBColor I;

  PointLight(const Point3f& p, const RGBColor& I) : Light(light_flag_e::point), pLight(p), I(I) {}
  RGBColor sample_Li(const Surfel& hit, Vector3f* wi, VisibilityTester* vis) override;
};

class DirectionalLight : public Light {
public:
  Vector3f dir;
  RGBColor I;

  DirectionalLight(const Vector3f& d, const RGBColor& I) : Light(light_flag_e::directional), dir(normalize(d)), I(I) {}
  RGBColor sample_Li(const Surfel& hit, Vector3f* wi, VisibilityTester* vis) override;
};

class AmbientLight : public Light {
public:
  RGBColor I;

  AmbientLight(const RGBColor& I) : Light(light_flag_e::ambient), I(I) {}
  RGBColor sample_Li(const Surfel& hit, Vector3f* wi, VisibilityTester* vis) override;
};

} // namespace ryt

#endif // LIGHT_HPP