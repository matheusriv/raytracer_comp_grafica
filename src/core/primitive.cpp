#include "primitive.hpp"

#include <algorithm>
#include <limits>

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

Bounds3f BVHAccel::compute_bounds(const std::vector<std::shared_ptr<Primitive>>& prims,
                                 int start,
                                 int end) {
  Bounds3f bounds;
  bounds.p_min = Point3f{std::numeric_limits<real_type>::infinity(),
                         std::numeric_limits<real_type>::infinity(),
                         std::numeric_limits<real_type>::infinity()};
  bounds.p_max = Point3f{-std::numeric_limits<real_type>::infinity(),
                         -std::numeric_limits<real_type>::infinity(),
                         -std::numeric_limits<real_type>::infinity()};
  for (int i = start; i < end; ++i) {
    const Bounds3f prim_bounds = prims[i]->world_bounds();
    bounds.p_min = min(bounds.p_min, prim_bounds.p_min);
    bounds.p_max = max(bounds.p_max, prim_bounds.p_max);
  }
  return bounds;
}

Point3f BVHAccel::centroid(const Bounds3f& bounds) {
  return Point3f{(bounds.p_min.x + bounds.p_max.x) * 0.5f,
                 (bounds.p_min.y + bounds.p_max.y) * 0.5f,
                 (bounds.p_min.z + bounds.p_max.z) * 0.5f};
}

bool BVHAccel::intersect_bounds(const Bounds3f& bounds, const Rayf& ray) {
  real_type t_min = ray.t_min;
  real_type t_max = ray.t_max;

  for (int axis = 0; axis < 3; ++axis) {
    real_type inv_d = 1.0f / ray.d[axis];
    real_type t0 = (bounds.p_min[axis] - ray.o[axis]) * inv_d;
    real_type t1 = (bounds.p_max[axis] - ray.o[axis]) * inv_d;
    if (inv_d < 0.0f) std::swap(t0, t1);
    t_min = std::max(t_min, t0);
    t_max = std::min(t_max, t1);
    if (t_max <= t_min) {
      return false;
    }
  }
  return true;
}

std::unique_ptr<BVHAccel::Node> BVHAccel::build_node(std::vector<std::shared_ptr<Primitive>>& prims,
                                                   int start,
                                                   int end,
                                                   int max_prims_per_node,
                                                   const std::string& split_method) {
  auto node = std::make_unique<Node>();
  node->start = start;
  node->end = end;
  node->bounds = compute_bounds(prims, start, end);

  int prim_count = end - start;
  if (prim_count <= max_prims_per_node) {
    node->is_leaf = true;
    return node;
  }

  Bounds3f centroid_bounds;
  centroid_bounds.p_min = Point3f{std::numeric_limits<real_type>::infinity(),
                                 std::numeric_limits<real_type>::infinity(),
                                 std::numeric_limits<real_type>::infinity()};
  centroid_bounds.p_max = Point3f{-std::numeric_limits<real_type>::infinity(),
                                 -std::numeric_limits<real_type>::infinity(),
                                 -std::numeric_limits<real_type>::infinity()};
  for (int i = start; i < end; ++i) {
    Bounds3f pb = prims[i]->world_bounds();
    Point3f c = centroid(pb);
    centroid_bounds.p_min = min(centroid_bounds.p_min, c);
    centroid_bounds.p_max = max(centroid_bounds.p_max, c);
  }

  Vector3f extent = centroid_bounds.p_max - centroid_bounds.p_min;
  int axis = max_dimension(extent);

  if (axis < 0 || axis > 2) {
    axis = 0;
  }

  auto comparator = [axis](const std::shared_ptr<Primitive>& a,
                           const std::shared_ptr<Primitive>& b) {
    Point3f ca = centroid(a->world_bounds());
    Point3f cb = centroid(b->world_bounds());
    return ca[axis] < cb[axis];
  };

  if (split_method == "middle") {
    real_type mid = 0.5f * (centroid_bounds.p_min[axis] + centroid_bounds.p_max[axis]);
    auto mid_it = std::partition(prims.begin() + start, prims.begin() + end,
                                 [&](const std::shared_ptr<Primitive>& prim) {
                                   return centroid(prim->world_bounds())[axis] < mid;
                                 });
    int mid_index = static_cast<int>(mid_it - prims.begin());
    if (mid_index == start || mid_index == end) {
      std::sort(prims.begin() + start, prims.begin() + end, comparator);
      mid_index = start + prim_count / 2;
    }
    node->left = build_node(prims, start, mid_index, max_prims_per_node, split_method);
    node->right = build_node(prims, mid_index, end, max_prims_per_node, split_method);
  } else {
    std::sort(prims.begin() + start, prims.begin() + end, comparator);
    int mid_index = start + prim_count / 2;
    node->left = build_node(prims, start, mid_index, max_prims_per_node, split_method);
    node->right = build_node(prims, mid_index, end, max_prims_per_node, split_method);
  }

  return node;
}

BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>> prims,
                   int max_prims_per_node,
                   std::string split_method)
    : primitives(std::move(prims)) {
  if (max_prims_per_node < 1) {
    max_prims_per_node = 1;
  }
  if (split_method != "middle" && split_method != "equal_counts") {
    split_method = "middle";
  }
  root = build_node(primitives, 0, static_cast<int>(primitives.size()), max_prims_per_node, split_method);
}

Bounds3f BVHAccel::world_bounds() const {
  return root ? root->bounds : Bounds3f();
}

bool BVHAccel::traverse(const Node* node, const Rayf& r, Surfel* sf, bool any_hit) const {
  if (!node || !intersect_bounds(node->bounds, r)) {
    return false;
  }

  if (node->is_leaf) {
    bool hit = false;
    for (int i = node->start; i < node->end; ++i) {
      if (any_hit) {
        if (primitives[i]->intersect_p(r)) {
          return true;
        }
      } else {
        if (primitives[i]->intersect(r, sf)) {
          hit = true;
        }
      }
    }
    return hit;
  }

  bool hit_left = traverse(node->left.get(), r, sf, any_hit);
  bool hit_right = traverse(node->right.get(), r, sf, any_hit);
  return hit_left || hit_right;
}

bool BVHAccel::intersect(const Rayf& r, Surfel* sf) const {
  return traverse(root.get(), r, sf, false);
}

bool BVHAccel::intersect_p(const Rayf& r) const {
  return traverse(root.get(), r, nullptr, true);
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

TransformedPrimitive::TransformedPrimitive(std::shared_ptr<Primitive> primitive, const Transform* PrimitiveToWorld)
    : primitive(std::move(primitive)), PrimitiveToWorld(PrimitiveToWorld) {}

Bounds3f TransformedPrimitive::world_bounds() const {
  return (*PrimitiveToWorld)(primitive->world_bounds());
}

bool TransformedPrimitive::intersect(const Rayf& r, Surfel* sf) const {
  Transform WorldToPrimitive = PrimitiveToWorld->inverse();
  Rayf ray = WorldToPrimitive(r);
  if (!primitive->intersect(ray, sf)) return false;
  r.t_max = ray.t_max;
  if (sf) {
    sf->p = (*PrimitiveToWorld)(sf->p);
    sf->n = Vector3f(normalize((*PrimitiveToWorld)(Normal3f(sf->n))));
    sf->wo = -r.d;
  }
  return true;
}

bool TransformedPrimitive::intersect_p(const Rayf& r) const {
  Transform WorldToPrimitive = PrimitiveToWorld->inverse();
  return primitive->intersect_p(WorldToPrimitive(r));
}

const Material* TransformedPrimitive::get_material() const {
  return primitive->get_material();
}

}  // namespace ryt