#ifndef BLINN_PHONG_INTEGRATOR_HPP
#define BLINN_PHONG_INTEGRATOR_HPP

#include "integrator.hpp"

namespace ryt {

class BlinnPhongIntegrator : public SamplerIntegrator {
public:
  BlinnPhongIntegrator(std::shared_ptr<Camera> cam) : SamplerIntegrator(std::move(cam)) {}
  virtual ~BlinnPhongIntegrator() = default;

  std::optional<RGBColor> Li(const Rayf& ray, const Scene& scene) const override;
};

} // namespace ryt

#endif // BLINN_PHONG_INTEGRATOR_HPP