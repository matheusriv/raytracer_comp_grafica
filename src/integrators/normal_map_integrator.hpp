#ifndef NORMAL_MAP_INTEGRATOR_HPP
#define NORMAL_MAP_INTEGRATOR_HPP

#include "integrator.hpp"

namespace ryt {

class NormalMapIntegrator : public SamplerIntegrator {
public:
  NormalMapIntegrator(std::shared_ptr<Camera> cam, int max_depth = 1) : SamplerIntegrator(std::move(cam), max_depth) {}
  std::optional<RGBColor> Li(const Rayf& ray, const Scene& scene, int depth = 0) const override;
};

} // namespace ryt
#endif // NORMAL_MAP_INTEGRATOR_HPP