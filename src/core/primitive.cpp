#include "primitive.hpp"

namespace ryt {

const Material* AggregatePrimitive::get_material() const {
  return nullptr;
}

PrimList::PrimList(std::vector<std::shared_ptr<Primitive>> prims)
    : primitives(std::move(prims)) {}

Bounds3f PrimList::world_bounds() const {
  Bounds3f b;
  for (const auto& prim : primitives) {
    b = Union(b, prim->world_bounds());
  }
  return b;
}

bool PrimList::intersect(const Rayf& r, Surfel* sf) const {
  bool hit_anything = false;
  for (const auto& prim : primitives) {
    if (prim->intersect(r, sf)) {
      hit_anything = true;
    }
  }
  return hit_anything;
}

bool PrimList::intersect_p(const Rayf& r) const {
  for (const auto& prim : primitives) {
    if (prim->intersect_p(r)) {
      return true;
    }
  }
  return false;
}

GeometricPrimitive::GeometricPrimitive(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material)
    : shape(std::move(shape)), material(std::move(material)) {}

Bounds3f GeometricPrimitive::world_bounds() const {
  return shape->world_bounds();
}

bool GeometricPrimitive::intersect(const Rayf& r, Surfel* sf) const {
  real_type t_hit;
  if (!shape->intersect(r, &t_hit, sf)) return false;
  r.t_max = t_hit;
  if (sf) sf->primitive = this;
  return true;
}

bool GeometricPrimitive::intersect_p(const Rayf& r) const {
  return shape->intersect_p(r);
}

const Material* GeometricPrimitive::get_material() const {
  return material.get();
}

void GeometricPrimitive::set_material(std::shared_ptr<Material> mat) {
  material = std::move(mat);
}

}  // namespace ryt