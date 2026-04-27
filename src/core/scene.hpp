#ifndef SCENE_HPP
#define SCENE_HPP

#include <memory>
#include <vector>

#include "background.hpp"
#include "primitive.hpp"

namespace ryt {

class Scene {
public:
  std::unique_ptr<Background> background;
  std::shared_ptr<Primitive> aggregate;

  explicit Scene(std::unique_ptr<Background> bg) : background(std::move(bg)) {}

  Scene(std::unique_ptr<Background> bg, std::shared_ptr<Primitive> agg)
      : background(std::move(bg)), aggregate(std::move(agg)) {}

  bool intersect(const Rayf& r, Surfel* sf) const {
    if (aggregate) return aggregate->intersect(r, sf);
    return false;
  }
  
  bool intersect_p(const Rayf& r) const {
    if (aggregate) return aggregate->intersect_p(r);
    return false;
  }
};

} // namespace ryt

#endif