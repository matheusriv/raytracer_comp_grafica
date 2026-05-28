#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <memory>
#include <string>
#include <vector>

#include "geometry.hpp"
#include "paramset.hpp"
#include "shape.hpp"
#include "surfel.hpp"

namespace ryt {

/// This struct implements an indexed triangle mesh database.
struct TriangleMesh {
  int n_triangles = 0;

  std::vector<int> vertex_indices;   ///< Per-triangle vertex index list.
  std::vector<int> normal_indices;   ///< Per-triangle normal index list.
  std::vector<int> uvcoord_indices;  ///< Per-triangle UV index list.

  std::vector<Point3f> vertices;  //!< The 3D geometric coordinates.
  std::vector<Normal3f> normals;  //!< The 3D normals.
  std::vector<Point2f> uvcoords;  //!< The 2D texture coordinates.

  TriangleMesh() = default;
  TriangleMesh(const TriangleMesh&) = delete;
  TriangleMesh& operator=(const TriangleMesh&) = delete;
  TriangleMesh(TriangleMesh&& other) = delete;
};

/// Represents a single triangle.
class Triangle : public Shape {
private:
  int* v;
  int* n;
  int* uv;
  std::shared_ptr<TriangleMesh> mesh;
  bool backface_cull;

public:
  Triangle(std::shared_ptr<TriangleMesh> mesh, int tri_id,
           bool backface_cull = true, bool flip_normals = false);

  Bounds3f world_bounds() const override;
  bool intersect(const Rayf& r, real_type* t_hit, Surfel* sf) const override;
  bool intersect_p(const Rayf& r) const override;

  friend std::ostream& operator<<(std::ostream& os, const Triangle& t);
};

std::vector<std::shared_ptr<Shape>> create_triangle_mesh_shape(const ParamSet& ps);
std::vector<std::shared_ptr<Shape>> create_triangle_mesh(std::shared_ptr<TriangleMesh> mesh,
                                                        bool backface_cull,
                                                        bool flip_normals);
bool load_mesh_data(const std::string& filename,
                    bool reverse_order,
                    bool compute_normals,
                    std::shared_ptr<TriangleMesh> mesh);

}  // namespace ryt

#endif  // TRIANGLE_HPP
