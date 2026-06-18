#include "triangle.hpp"

#include <algorithm>
#include <iostream>

#include "common.hpp"
#include "geometry.hpp"
#include "surfel.hpp"
#include "shape.hpp"
#include "tiny_obj_loader.h"

namespace ryt {

Triangle::Triangle(std::shared_ptr<TriangleMesh> mesh, int tri_id,
                   bool backface_cull, bool flip_normals)
    : Shape(nullptr, nullptr, flip_normals), v{ &mesh->vertex_indices[3 * tri_id] },
      n{ &mesh->normal_indices[3 * tri_id] },
      uv{ &mesh->uvcoord_indices[3 * tri_id] },
      mesh{ std::move(mesh) }, backface_cull{ backface_cull } {
}

Bounds3f Triangle::world_bounds() const {
  const Point3f& p0 = mesh->vertices[v[0]];
  const Point3f& p1 = mesh->vertices[v[1]];
  const Point3f& p2 = mesh->vertices[v[2]];
  Point3f pmin = min(min(p0, p1), p2);
  Point3f pmax = max(max(p0, p1), p2);
  
  const float epsilon = 1e-4f;
  return Bounds3f(Point3f{pmin[0] - epsilon, pmin[1] - epsilon, pmin[2] - epsilon},
                  Point3f{pmax[0] + epsilon, pmax[1] + epsilon, pmax[2] + epsilon});
}

bool Triangle::intersect_p(const Rayf& r) const {
  // Reuses the core logic of intersect, passing nullptr to skip
  // the heavy computation of normals, UVs, and Surfel generation.
  return intersect(r, nullptr, nullptr);
}

bool Triangle::intersect(const Rayf& r, real_type* t_hit, Surfel* sf) const {
  const Point3f& p0 = mesh->vertices[v[0]];
  const Point3f& p1 = mesh->vertices[v[1]];
  const Point3f& p2 = mesh->vertices[v[2]];

  Vector3f e1 = p1 - p0;
  Vector3f e2 = p2 - p0;
  Vector3f pvec = cross(r.d, e2);
  real_type det = dot(e1, pvec);

  const real_type epsilon = 1e-8f;
  if (backface_cull) {
    if (det > -epsilon) return false;
  } else {
    if (std::abs(det) < epsilon) return false;
  }

  real_type inv_det = 1.0f / det;
  Vector3f tvec = r.o - p0;
  real_type u_coord = dot(tvec, pvec) * inv_det;
  if (u_coord < 0.0f || u_coord > 1.0f) return false;

  Vector3f qvec = cross(tvec, e1);
  real_type v_coord = dot(r.d, qvec) * inv_det;
  if (v_coord < 0.0f || u_coord + v_coord > 1.0f) return false;

  real_type t = dot(e2, qvec) * inv_det;
  if (t < r.t_min || t > r.t_max) return false;

  if (t_hit) *t_hit = t;
  if (sf != nullptr) {
    sf->p = r(t);
    Vector3f shading_normal;
    if (!mesh->normals.empty() && n[0] >= 0 && n[1] >= 0 && n[2] >= 0) {
      const Normal3f& n0 = mesh->normals[n[0]];
      const Normal3f& n1 = mesh->normals[n[1]];
      const Normal3f& n2 = mesh->normals[n[2]];
      shading_normal = normalize((1.0f - u_coord - v_coord) * Vector3f(n0) + u_coord * Vector3f(n1) + v_coord * Vector3f(n2));
    } else {
      shading_normal = normalize(-cross(e1, e2));
    }
    if (flip_normals) shading_normal = -shading_normal;
    sf->n = shading_normal;
    sf->wo = -r.d;
    sf->time = t;

    if (!mesh->uvcoords.empty() && uv[0] >= 0 && uv[1] >= 0 && uv[2] >= 0) {
      const Point2f& uv0 = mesh->uvcoords[uv[0]];
      const Point2f& uv1 = mesh->uvcoords[uv[1]];
      const Point2f& uv2 = mesh->uvcoords[uv[2]];
      sf->uv = Point2f{
        (1.0f - u_coord - v_coord) * uv0.x + u_coord * uv1.x + v_coord * uv2.x,
        (1.0f - u_coord - v_coord) * uv0.y + u_coord * uv1.y + v_coord * uv2.y};
    } else {
      sf->uv = Point2f{ u_coord, v_coord };
    }
  }
  return true;
}

std::ostream& operator<<(std::ostream& os, const Triangle& t) {
  os << "Triangle(indices=[" << t.v[0] << ", " << t.v[1] << ", " << t.v[2] << "], backface_cull="
     << std::boolalpha << t.backface_cull << ")";
  return os;
}

static void ensure_uvcoords(TriangleMesh& mesh) {
  if (mesh.uvcoords.empty()) {
    mesh.uvcoords.emplace_back(Point2f{0.0f, 0.0f});
  }
}

static void reverse_triangle_indices(int* array) {
  std::swap(array[0], array[2]);
}

static void reverse_triangles(TriangleMesh& mesh) {
  for (int tri = 0; tri < mesh.n_triangles; ++tri) {
    reverse_triangle_indices(&mesh.vertex_indices[3 * tri]);
    reverse_triangle_indices(&mesh.normal_indices[3 * tri]);
    reverse_triangle_indices(&mesh.uvcoord_indices[3 * tri]);
  }
}

static void compute_triangle_normals(TriangleMesh& mesh) {
  mesh.normals.clear();
  mesh.normal_indices.assign(mesh.n_triangles * 3, 0);
  for (int tri = 0; tri < mesh.n_triangles; ++tri) {
    int* vertex_indices = &mesh.vertex_indices[3 * tri];
    const Point3f& p0 = mesh.vertices[vertex_indices[0]];
    const Point3f& p1 = mesh.vertices[vertex_indices[1]];
    const Point3f& p2 = mesh.vertices[vertex_indices[2]];
    Vector3f normal = normalize(-cross(p1 - p0, p2 - p0));
    mesh.normals.emplace_back(normal);
    int base_index = static_cast<int>(mesh.normals.size()) - 1;
    mesh.normal_indices[3 * tri + 0] = base_index;
    mesh.normal_indices[3 * tri + 1] = base_index;
    mesh.normal_indices[3 * tri + 2] = base_index;
  }
}

bool load_mesh_data(const std::string& filename,
                    bool reverse_order,
                    bool compute_normals_flag,
                    std::shared_ptr<TriangleMesh> mesh,
                    const Transform* o2w) {
  if (!mesh) return false;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn;
  std::string err;

  bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());
  if (!warn.empty()) {
    WARNING("tinyobj warning: " + warn);
  }
  if (!err.empty()) {
    WARNING("tinyobj error: " + err);
  }
  if (!result) {
    WARNING("Unable to parse OBJ file: " + filename);
    return false;
  }

  mesh->vertices.clear();
  mesh->normals.clear();
  mesh->uvcoords.clear();
  mesh->vertex_indices.clear();
  mesh->normal_indices.clear();
  mesh->uvcoord_indices.clear();

  for (size_t i = 0; i + 2 < attrib.vertices.size(); i += 3) {
    mesh->vertices.emplace_back(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
  }

  for (size_t i = 0; i + 2 < attrib.normals.size(); i += 3) {
    mesh->normals.emplace_back(attrib.normals[i], attrib.normals[i + 1], attrib.normals[i + 2]);
  }

  for (size_t i = 0; i + 1 < attrib.texcoords.size(); i += 2) {
    mesh->uvcoords.emplace_back(attrib.texcoords[i], attrib.texcoords[i + 1]);
  }

  for (const auto& shape : shapes) {
    size_t index_offset = 0;
    for (size_t face = 0; face < shape.mesh.num_face_vertices.size(); ++face) {
      int fv = shape.mesh.num_face_vertices[face];
      if (fv < 3) {
        index_offset += fv;
        continue;
      }

      for (int vert = 0; vert + 2 < fv; ++vert) {
        const tinyobj::index_t& i0 = shape.mesh.indices[index_offset + 0];
        const tinyobj::index_t& i1 = shape.mesh.indices[index_offset + vert + 1];
        const tinyobj::index_t& i2 = shape.mesh.indices[index_offset + vert + 2];

        mesh->vertex_indices.push_back(i0.vertex_index);
        mesh->vertex_indices.push_back(i1.vertex_index);
        mesh->vertex_indices.push_back(i2.vertex_index);

        if (!attrib.texcoords.empty() && i0.texcoord_index >= 0 && i1.texcoord_index >= 0 && i2.texcoord_index >= 0) {
          mesh->uvcoord_indices.push_back(i0.texcoord_index);
          mesh->uvcoord_indices.push_back(i1.texcoord_index);
          mesh->uvcoord_indices.push_back(i2.texcoord_index);
        } else {
          mesh->uvcoord_indices.push_back(0);
          mesh->uvcoord_indices.push_back(0);
          mesh->uvcoord_indices.push_back(0);
        }

        if (!attrib.normals.empty() && i0.normal_index >= 0 && i1.normal_index >= 0 && i2.normal_index >= 0) {
          mesh->normal_indices.push_back(i0.normal_index);
          mesh->normal_indices.push_back(i1.normal_index);
          mesh->normal_indices.push_back(i2.normal_index);
        } else {
          mesh->normal_indices.push_back(0);
          mesh->normal_indices.push_back(0);
          mesh->normal_indices.push_back(0);
        }
      }

      index_offset += fv;
    }
  }

  if (mesh->vertex_indices.empty()) {
    WARNING("OBJ file has no triangle data: " + filename);
    return false;
  }

  mesh->n_triangles = static_cast<int>(mesh->vertex_indices.size() / 3);
  if (mesh->n_triangles * 3 != static_cast<int>(mesh->vertex_indices.size())) {
    WARNING("OBJ file produced invalid triangle index count: " + filename);
    return false;
  }

  if (mesh->uvcoord_indices.empty()) {
    mesh->uvcoord_indices.assign(mesh->n_triangles * 3, 0);
  }
  if (mesh->normal_indices.empty()) {
    mesh->normal_indices.assign(mesh->n_triangles * 3, 0);
  }

  if (o2w) {
    for (auto& v : mesh->vertices) {
      v = (*o2w)(v);
    }
    for (auto& n : mesh->normals) {
      n = Normal3f(normalize(Vector3f((*o2w)(n))));
    }
  }

  if (reverse_order) {
    reverse_triangles(*mesh);
  }

  if (compute_normals_flag || mesh->normals.empty()) {
    compute_triangle_normals(*mesh);
  }

  if (mesh->uvcoords.empty()) {
    ensure_uvcoords(*mesh);
  }

  return true;
}

static bool extract_triangle_mesh_data(const ParamSet& ps,
                                       bool reverse_order,
                                       bool compute_normals_flag,
                                       std::shared_ptr<TriangleMesh> mesh,
                                       const Transform* o2w) {
  if (!mesh) return false;

  int ntriangles = ps.retrieve<int>("ntriangles", 0);
  if (ntriangles <= 0) {
    WARNING("trianglemesh requires ntriangles > 0");
    return false;
  }

  auto indices = ps.retrieve<std::vector<int>>("vertex_indices", {});
  if (indices.empty()) {
    indices = ps.retrieve<std::vector<int>>("indices", {});
  }
  auto vertices = ps.retrieve<std::vector<Point3f>>("vertices", {});
  auto normals = ps.retrieve<std::vector<Normal3f>>("normals", {});
  auto normal_indices = ps.retrieve<std::vector<int>>("normal_indices", {});
  auto uvcoords = ps.retrieve<std::vector<Point2f>>("uv", {});

  if (indices.empty() || static_cast<int>(indices.size()) != ntriangles * 3) {
    WARNING("trianglemesh requires exactly 3*ntriangles vertex_indices");
    return false;
  }
  if (vertices.empty()) {
    WARNING("trianglemesh requires vertex positions");
    return false;
  }

  mesh->n_triangles = ntriangles;
  mesh->vertices = std::move(vertices);
  mesh->vertex_indices = std::move(indices);
  mesh->uvcoord_indices.assign(ntriangles * 3, 0);
  mesh->normal_indices.assign(ntriangles * 3, 0);

  if (!normals.empty()) {
    mesh->normals = std::move(normals);
    if (!normal_indices.empty()) {
      if (static_cast<int>(normal_indices.size()) != ntriangles * 3) {
        WARNING("trianglemesh requires exactly 3*ntriangles normal_indices");
        return false;
      }
      mesh->normal_indices = std::move(normal_indices);
    } else if (static_cast<int>(mesh->normals.size()) == static_cast<int>(mesh->vertices.size())) {
      for (int idx = 0; idx < ntriangles * 3; ++idx) {
        mesh->normal_indices[idx] = mesh->vertex_indices[idx];
      }
    } else if (static_cast<int>(mesh->normals.size()) == ntriangles * 3) {
      for (int idx = 0; idx < ntriangles * 3; ++idx) {
        mesh->normal_indices[idx] = idx;
      }
    } else if (static_cast<int>(mesh->normals.size()) == ntriangles) {
      for (int tri = 0; tri < ntriangles; ++tri) {
        int entry = tri * 3;
        mesh->normal_indices[entry + 0] = tri;
        mesh->normal_indices[entry + 1] = tri;
        mesh->normal_indices[entry + 2] = tri;
      }
    } else {
      WARNING("trianglemesh normals size does not match vertices or triangles");
      return false;
    }
  }

  if (!uvcoords.empty()) {
    mesh->uvcoords = std::move(uvcoords);
    if (static_cast<int>(mesh->uvcoords.size()) == static_cast<int>(mesh->vertices.size())) {
      for (int idx = 0; idx < ntriangles * 3; ++idx) {
        mesh->uvcoord_indices[idx] = mesh->vertex_indices[idx];
      }
    } else if (static_cast<int>(mesh->uvcoords.size()) == ntriangles * 3) {
      for (int idx = 0; idx < ntriangles * 3; ++idx) {
        mesh->uvcoord_indices[idx] = idx;
      }
    } else if (static_cast<int>(mesh->uvcoords.size()) == ntriangles) {
      for (int tri = 0; tri < ntriangles; ++tri) {
        int entry = tri * 3;
        mesh->uvcoord_indices[entry + 0] = tri;
        mesh->uvcoord_indices[entry + 1] = tri;
        mesh->uvcoord_indices[entry + 2] = tri;
      }
    } else {
      WARNING("trianglemesh uv size does not match vertices or triangles");
      return false;
    }
  }

  if (mesh->uvcoords.empty()) {
    ensure_uvcoords(*mesh);
  }

  if (o2w) {
    for (auto& v : mesh->vertices) {
      v = (*o2w)(v);
    }
    for (auto& n : mesh->normals) {
      n = Normal3f(normalize(Vector3f((*o2w)(n))));
    }
  }

  if (reverse_order) {
    reverse_triangles(*mesh);
  }

  if (compute_normals_flag || mesh->normals.empty()) {
    compute_triangle_normals(*mesh);
  }

  return true;
}

std::vector<std::shared_ptr<Shape>> create_triangle_mesh(std::shared_ptr<TriangleMesh> mesh,
                                                        bool backface_cull,
                                                        bool flip_normals) {
  std::vector<std::shared_ptr<Shape>> result;
  if (!mesh || mesh->n_triangles <= 0) return result;
  result.reserve(mesh->n_triangles);
  for (int tri = 0; tri < mesh->n_triangles; ++tri) {
    result.push_back(std::make_shared<Triangle>(mesh, tri, backface_cull, flip_normals));
  }
  return result;
}

std::vector<std::shared_ptr<Shape>> create_triangle_mesh_shape(const ParamSet& ps, const Transform* o2w) {
  bool reverse_vertex_order = ps.retrieve<bool>("reverse_vertex_order", false);
  bool compute_normals_flag = ps.retrieve<bool>("compute_normals", false);
  bool backface_cull = ps.retrieve<bool>("backface_cull", true);

  auto mesh = std::make_shared<TriangleMesh>();
  std::string filename = ps.retrieve<std::string>("filename", "");
  bool loaded = false;
  if (!filename.empty()) {
    loaded = load_mesh_data(filename, reverse_vertex_order, compute_normals_flag, mesh, o2w);
  } else {
    loaded = extract_triangle_mesh_data(ps, reverse_vertex_order, compute_normals_flag, mesh, o2w);
  }

  if (!loaded) {
    return {};
  }
  bool flip_normals = ps.retrieve<bool>("flip_normals", false);
  return create_triangle_mesh(mesh, backface_cull, flip_normals);
}

}  // namespace ryt
