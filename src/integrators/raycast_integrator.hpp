#ifndef RAYCAST_INTEGRATOR_HPP
#define RAYCAST_INTEGRATOR_HPP

#include "integrator.hpp"

namespace ryt {

/// A simple integrator that returns the unlit material color
class RayCastIntegrator : public SamplerIntegrator {
public:
  RayCastIntegrator(std::shared_ptr<Camera> cam) : SamplerIntegrator(std::move(cam)) {}
  std::optional<RGBColor> Li(const Rayf& ray, const Scene& scene) const override;
};

} // namespace ryt

#endif // RAYCAST_INTEGRATOR_HPP