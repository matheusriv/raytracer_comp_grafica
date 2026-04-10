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
  std::vector<std::shared_ptr<Primitive>> primitives;

  explicit Scene(std::unique_ptr<Background> bg) : background(std::move(bg)) {}

  Scene(std::unique_ptr<Background> bg, std::vector<std::shared_ptr<Primitive>> prims)
      : background(std::move(bg)), primitives(std::move(prims)) {}
};

} // namespace ryt

#endif