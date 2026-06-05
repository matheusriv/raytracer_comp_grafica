#include "integrator.hpp"
#include <iostream>

namespace ryt {

Integrator* create_integrator(std::shared_ptr<Camera> camera, const ParamSet& ps) {
  auto type = ps.retrieve<std::string>("type", "flat");
  int max_depth = ps.retrieve<int>("depth", 1);

  if (type == "flat") {
    return new FlatIntegrator(std::move(camera));
  }
  if (type == "normal_map") {
    return new NormalMapIntegrator(std::move(camera));
  }
  if (type == "blinn_phong") {
    return new BlinnPhongIntegrator(std::move(camera), max_depth);
  }
  if (type == "toon") {
    auto intervals = ps.retrieve<std::vector<real_type>>("mapping_interval", {});
    return new ToonIntegrator(std::move(camera), intervals);
  }
  std::cerr << "Warning: Unknown integrator type '" << type << "', falling back to 'flat'.\n";
  return new FlatIntegrator(std::move(camera));
}

} // namespace ryt