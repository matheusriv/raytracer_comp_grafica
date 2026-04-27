#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include <memory>
#include <optional>
#include "../core/scene.hpp"
#include "../core/camera.hpp"
#include "../core/paramset.hpp"

namespace ryt {

/// Base class for all Integrators
class Integrator {
public:
  std::shared_ptr<Camera> camera;

  Integrator(std::shared_ptr<Camera> cam) : camera(std::move(cam)) {}
  virtual ~Integrator() = default;
  virtual void render(const Scene& scene) = 0;
};

/// Base class for integrators that sample rays from the camera
class SamplerIntegrator : public Integrator {
public:
  SamplerIntegrator(std::shared_ptr<Camera> cam) : Integrator(std::move(cam)) {}
  virtual ~SamplerIntegrator() = default;

  virtual void preprocess(const Scene& scene);
  void render(const Scene& scene) override;
  virtual std::optional<RGBColor> Li(const Rayf& ray, const Scene& scene) const = 0;
};

/// Factory function to create integrators
Integrator* create_integrator(std::shared_ptr<Camera> camera, const ParamSet& ps);

} // namespace ryt

#endif