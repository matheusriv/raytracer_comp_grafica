#ifndef NORMAL_MAP_INTEGRATOR_HPP
#define NORMAL_MAP_INTEGRATOR_HPP

#include "integrator.hpp"

namespace ryt {

class NormalMapIntegrator : public SamplerIntegrator {
public:
  NormalMapIntegrator(std::shared_ptr<Camera> cam) : SamplerIntegrator(std::move(cam)) {}
  std::optional<RGBColor> Li(const Rayf& ray, const Scene& scene) const override;
};

} // namespace ryt
#endif // NORMAL_MAP_INTEGRATOR_HPP