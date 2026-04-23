#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include <memory>
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

/// A simple integrator that returns the unlit material color
class FlatIntegrator : public Integrator {
public:
  FlatIntegrator(std::shared_ptr<Camera> cam) : Integrator(std::move(cam)) {}
  void render(const Scene& scene) override;
};

/// Factory function to create integrators
Integrator* create_integrator(std::shared_ptr<Camera> camera, const ParamSet& ps);

} // namespace ryt

#endif