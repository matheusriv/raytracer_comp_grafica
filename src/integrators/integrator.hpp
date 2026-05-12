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
  virtual std::optional<RGBColor> Li(const Rayf& ray, const Scene& scene, int depth = 0) const = 0;
};

class BlinnPhongIntegrator : public SamplerIntegrator {
public:
  int max_depth;

  BlinnPhongIntegrator(std::shared_ptr<Camera> cam, int max_depth = 1) : SamplerIntegrator(std::move(cam)), max_depth(max_depth) {}
  virtual ~BlinnPhongIntegrator() = default;

  std::optional<RGBColor> Li(const Rayf& ray, const Scene& scene, int depth = 0) const override;
};

class NormalMapIntegrator : public SamplerIntegrator {
public:
  NormalMapIntegrator(std::shared_ptr<Camera> cam) : SamplerIntegrator(std::move(cam)) {}
  std::optional<RGBColor> Li(const Rayf& ray, const Scene& scene, int depth = 0) const override;
};

/// A simple integrator that returns the unlit material color
class FlatIntegrator : public SamplerIntegrator {
public:
  FlatIntegrator(std::shared_ptr<Camera> cam) : SamplerIntegrator(std::move(cam)) {}
  std::optional<RGBColor> Li(const Rayf& ray, const Scene& scene, int depth = 0) const override;
};

/// Factory function to create integrators
Integrator* create_integrator(std::shared_ptr<Camera> camera, const ParamSet& ps);

} // namespace ryt

#endif