#ifndef SCENE_HPP
#define SCENE_HPP

#include <memory>

#include "background.hpp"

namespace ryt {

class Scene {
public:
  std::unique_ptr<Background> background;

  Scene(std::unique_ptr<Background> bg) : background(std::move(bg)) {}
};

} // namespace ryt

#endif