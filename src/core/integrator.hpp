#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include "scene.hpp"
#include "camera.hpp"
#include "paramset.hpp"

namespace ryt {

/// Base class for all Integrators
class Integrator {
public:
  virtual ~Integrator() = default;
  virtual void render(const Camera& camera, const Scene& scene) = 0;
};

/// A simple integrator that returns the unlit material color
class FlatIntegrator : public Integrator {
public:
  FlatIntegrator() = default;
  void render(const Camera& camera, const Scene& scene) override;
};

/// Factory function to create integrators
Integrator* create_integrator(const ParamSet& ps);

} // namespace ryt

#endif