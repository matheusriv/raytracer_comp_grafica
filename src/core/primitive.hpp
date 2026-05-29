#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include <algorithm>
#include <memory>
#include <vector>

#include "geometry.hpp"
#include "material.hpp"
#include "shape.hpp"
#include "surfel.hpp"

namespace ryt {

/// Abstract base class for any surface that a ray might intersect.
class Primitive {
public:
  virtual ~Primitive() = default;

  virtual Bounds3f world_bounds() const = 0;
  /// Computes the intersection of the ray with this primitive.
  virtual bool intersect(const Rayf& r, Surfel* sf) const = 0;
  /// Faster shadow-ray version: only returns true/false, does not fill Surfel.
  virtual bool intersect_p(const Rayf& r) const = 0;
  virtual const Material* get_material() const = 0;
};

class AggregatePrimitive : public Primitive {
public:
  const Material* get_material() const override;
};

class PrimList : public AggregatePrimitive {
public:
  explicit PrimList(std::vector<std::shared_ptr<Primitive>> prims);

  Bounds3f world_bounds() const override;
  bool intersect(const Rayf& r, Surfel* sf) const override;
  bool intersect_p(const Rayf& r) const override;

private:
  std::vector<std::shared_ptr<Primitive>> primitives;
};

class BVHAccel : public AggregatePrimitive {
public:
  BVHAccel(std::vector<std::shared_ptr<Primitive>> prims,
           int max_prims_per_node = 4,
           std::string split_method = "middle");

  Bounds3f world_bounds() const override;
  bool intersect(const Rayf& r, Surfel* sf) const override;
  bool intersect_p(const Rayf& r) const override;

private:
  struct Node {
    Bounds3f bounds;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
    int start = 0;
    int end = 0;
    bool is_leaf = false;
  };

  static Bounds3f compute_bounds(const std::vector<std::shared_ptr<Primitive>>& prims,
                                 int start,
                                 int end);
  static Point3f centroid(const Bounds3f& bounds);
  static bool intersect_bounds(const Bounds3f& bounds, const Rayf& ray);
  std::unique_ptr<Node> build_node(std::vector<std::shared_ptr<Primitive>>& prims,
                                   int start,
                                   int end,
                                   int max_prims_per_node,
                                   const std::string& split_method);
  bool traverse(const Node* node, const Rayf& r, Surfel* sf, bool any_hit) const;

  std::vector<std::shared_ptr<Primitive>> primitives;
  std::unique_ptr<Node> root;
};

class GeometricPrimitive : public Primitive {
public:
  GeometricPrimitive(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material);

  Bounds3f world_bounds() const override;
  bool intersect(const Rayf& r, Surfel* sf) const override;
  bool intersect_p(const Rayf& r) const override;

  const Material* get_material() const override;
  void set_material(std::shared_ptr<Material> mat);

private:
  std::shared_ptr<Shape> shape;
  std::shared_ptr<Material> material;
};

}  // namespace ryt

#endif // PRIMITIVE_HPP
