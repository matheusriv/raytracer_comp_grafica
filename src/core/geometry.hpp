#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <cmath>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "../../msg_system/error.hpp"
#include "common.hpp"

// ======================================================================================
// Add this to ignore the presence anonymous struct and nested anonymous type warnings.
// --------------------------------------------------------------------------------------
#include <iterator>
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
// ======================================================================================

namespace ryt {

// forward declaration.
template <typename T>
class Vector2;

template <typename T>
class Point2 {
public:
  //== Point2 Public Data
  T x;  //!< X coordinate of the point.
  T y;  //!< Y coordinate of the point.

  //== Point2 Public Methods
  Point2() : x{ 0 }, y{ 0 } {}
  Point2(T xx, T yy) : x{ xx }, y{ yy } {}

  Point2<T>& operator=(const Point2<T>& p) = default;

  T operator[](int i) const {
    assert(i >= 0 and i <= 1);
    if (i == 0) {
      return x;
    }
    return y;
  }

  T& operator[](int i) {
    assert(i >= 0 and i <= 1);
    if (i == 0) {
      return x;
    }
    return y;
  }

  bool operator==(const Point2<T>& p) const { return x == p.x and y == p.y; }

  bool operator!=(const Point2<T>& p) const { return x != p.x or y != p.y; }

};  // Point2

template <typename T>
std::ostream& operator<<(std::ostream& os, const Point2<T>& v) {
  os << "[ " << v.x << ", " << v.y << " ]";
  return os;
}

// forward declaration.
template <typename T>
class Vector3;

template <typename T>
class Point3 {
public:
  //== Point3 Public Data
  T x;  //!< X coordinate of the point.
  T y;  //!< Y coordinate of the point.
  T z;  //!< Z coordinate of the point.

  //== Point3 Public Methods
  Point3() : x{ 0 }, y{ 0 }, z{ 0 } {}
  Point3(T x, T y, T z) : x{ x }, y{ y }, z{ z } {}
  Point3(const T& vet) : x{ vet[0] }, y{ vet[1] }, z{ vet[2] } { assert(not vet.has_nans()); }

  template <typename U>
  explicit Point3(const Point3<U>& p) : x{ (T)p.x }, y{ (T)p.y }, z{ (T)p.z } {}

  template <typename U>
  explicit operator Vector3<U>() const {
    return Vector3<U>{ x, y, z };
  }

  Point3(const Point3<T>& p) {
    x = p.x;
    y = p.y;
    z = p.z;
  }

  Point3<T>& operator=(const Point3<T>& p) = default;

  T operator[](int i) const {
    assert(i >= 0 and i <= 2);
    if (i == 0) {
      return x;
    }
    if (i == 1) {
      return y;
    }
    return z;
  }

  T& operator[](int i) {
    assert(i >= 0 and i <= 2);
    if (i == 0) {
      return x;
    }
    if (i == 1) {
      return y;
    }
    return z;
  }

  bool operator==(const Point3<T>& p) const { return x == p.x and y == p.y and z == p.z; }
  bool operator!=(const Point3<T>& p) const { return x != p.x || y != p.y || z != p.z; }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Point3<T>& v) {
  os << "[ " << v.x << ", " << v.y << ", " << v.z << " ]";
  return os;
}

// Vector Declarations
template <typename T>
class Vector2 {
public:
  //== Vector2 Public Data
  T x, y;

  //== Vector2 Public Methods
  Vector2() : x{ 0 }, y{ 0 } {}
  Vector2(T xx, T yy) : x{ xx }, y{ yy } {}
  Vector2(const T& vet) : x{ vet[0] }, y{ vet[1] } {}
  explicit Vector2(const Point2<T>& p) : x{ p.x }, y{ p.y } {};
  explicit Vector2(const Point3<T>& p) : x{ p.x }, y{ p.y } {};
  Vector2(const Vector2<T>& v) {
    x = v.x;
    y = v.y;
  }

  Vector2<T>& operator=(const Vector2<T>& v) = default;

  T operator[](int i) const {
    assert(i >= 0 and i <= 1);
    if (i == 0) {
      return x;
    }
    return y;
  }

  T& operator[](int i) {
    assert(i >= 0 and i <= 1);
    if (i == 0) {
      return x;
    }
    return y;
  }
};

// forward declaration.
template <typename T>
class Normal3;

template <typename T>
class Vector3 {
public:
  //== Vector3 Public Data
  union {
    struct {
      T x{}, y{}, z{};
    };
    struct {
      T r, g, b;
    };
    struct {
      T s, t, p;
    };
  };

  //== Vector3 Public Methods
  T operator[](int i) const {
    assert(i >= 0 and i <= 2);
    if (i == 0) {
      return x;
    }
    if (i == 1) {
      return y;
    }
    return z;
  }

  T& operator[](int i) {
    assert(i >= 0 and i <= 2);
    if (i == 0) {
      return x;
    }
    if (i == 1) {
      return y;
    }
    return z;
  }

  Vector3() = default;

  Vector3(T x, T y, T z) : x{ x }, y{ y }, z{ z } { assert(not has_nans()); }
  // Vector3(T x, T y, T z) : x{ x }, y{ y }, z{ z } { ; }

  [[nodiscard]]
  Vector3(const T* vet)
      : x(vet[0]), y(vet[1]), z(vet[2]) {
    assert(not has_nans());
  }

  [[nodiscard]] bool has_nans() const { return std::isnan(x) || std::isnan(y) || std::isnan(z); }

  Vector3(const Vector3<T>& v) {
    assert(not v.has_nans());
    x = v.x;
    y = v.y;
    z = v.z;
  }

  /// Converting a normal to a vector.
  explicit Vector3(const Normal3<T>& n) {
    assert(not n.has_nans());
    x = n.x;
    y = n.y;
    z = n.z;
  }

  /// Converting a normal to a vector.
  explicit Vector3(const Point3<T>& p) {
    assert(not p.has_nans());
    x = p.x;
    y = p.y;
    z = p.z;
  }

  Vector3<T>& operator=(const Vector3<T>& v) {
    assert(not v.has_nans());
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
  }

  bool operator==(const Vector3<T>& v) const { return x == v.x and y == v.y and z == v.z; }
  bool operator!=(const Vector3<T>& v) const { return x != v.x or y != v.y or z != v.z; }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Vector3<T>& v) {
  os << "[ " << v.x << ", " << v.y << ", " << v.z << " ]";
  return os;
}

/// Normal Declarations
template <typename T>
class Normal3 {
public:
  //== Normal3 Public Data
  T x, y, z;

  //== Normal3 Public Methods
  Normal3() : x{ 0 }, y{ 0 }, z{ 0 } {}
  Normal3(T xx, T yy, T zz) : x{ xx }, y{ yy }, z{ zz } { assert(not has_nans()); }
  Normal3(T xx) : x{ xx }, y{ xx }, z{ xx } { assert(not has_nans()); }

  [[nodiscard]] bool has_nans() const { return std::isnan(x) || std::isnan(y) || std::isnan(z); }

  Normal3<T>(const Normal3<T>& n) {
    assert(not n.has_nans());
    x = n.x;
    y = n.y;
    z = n.z;
  }

  Normal3<T>& operator=(const Normal3<T>& n) {
    assert(not n.has_nans());
    x = n.x;
    y = n.y;
    z = n.z;
    return *this;
  }

  explicit Normal3<T>(const Vector3<T>& v) : x{ v.x }, y{ v.y }, z{ v.z } {
    assert(not v.has_nans());
  }

  bool operator==(const Normal3<T>& n) const { return x == n.x and y == n.y and z == n.z; }
  bool operator!=(const Normal3<T>& n) const { return x != n.x or y != n.y or z != n.z; }

  T operator[](int i) const {
    assert(i >= 0 and i <= 2);
    if (i == 0) {
      return x;
    }
    if (i == 1) {
      return y;
    }
    return z;
  }

  T& operator[](int i) {
    assert(i >= 0 and i <= 2);
    if (i == 0) {
      return x;
    }
    if (i == 1) {
      return y;
    }
    return z;
  }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Normal3<T>& v) {
  os << "[ " << v.x << ", " << v.y << ", " << v.z << " ]";
  return os;
}

// == Declaring all types based on the previous template classes.
using Point2f = Point2<real_type>;
using Point2i = Point2<int>;
using Point3f = Point3<real_type>;
using Point3i = Point3<int>;
// using Vector2f = glm::vec2;
// using Vector2i = glm::ivec2;
// using Vector3f = glm::vec3;
// using Vector3i = glm::ivec3;
using Vector2f = Vector2<real_type>;
using Vector2i = Vector2<int>;
using Vector3f = Vector3<real_type>;
using Vector3i = Vector3<int>;
using Normal3f = Normal3<float>;

// Point3 and Vector3 operators
template <typename T>
Vector3<T> operator-(const Point3<T>& p1, const Point3<T>& p2) {
  return Vector3<T>(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
}

template <typename T>
Point3<T> operator+(const Point3<T>& p, const Vector3<T>& v) {
  return Point3<T>(p.x + v.x, p.y + v.y, p.z + v.z);
}

template <typename T>
Point3<T> operator-(const Point3<T>& p, const Vector3<T>& v) {
  return Point3<T>(p.x - v.x, p.y - v.y, p.z - v.z);
}

template <typename T>
Vector3<T> operator+(const Vector3<T>& v1, const Vector3<T>& v2) {
  return Vector3<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

template <typename T>
Vector3<T> operator-(const Vector3<T>& v1, const Vector3<T>& v2) {
  return Vector3<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

template <typename T, typename U>
Vector3<T> operator*(U s, const Vector3<T>& v) {
  return Vector3<T>(s * v.x, s * v.y, s * v.z);
}

template <typename T, typename U>
Vector3<T> operator*(const Vector3<T>& v, U s) {
  return s * v;
}

template <typename T, typename U>
Vector3<T> operator/(const Vector3<T>& v, U s) {
  return Vector3<T>(v.x / s, v.y / s, v.z / s);
}

template <typename T>
T dot(const Vector3<T>& v1, const Vector3<T>& v2) {
  assert(not v1.has_nans() and not v2.has_nans());
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename T>
T abs_dot(const Vector3<T>& v1, const Vector3<T>& v2) {
  assert(not v1.has_nans() and not v2.has_nans());
  return std::abs(dot(v1, v2));
}

template <typename T>
Vector3<T> cross(const Vector3<T>& v1, const Vector3<T>& v2) {
  assert(not v1.has_nans() and not v2.has_nans());
  return { (v1.z * v2.y) - (v1.y * v2.z),
           (v1.x * v2.z) - (v1.z * v2.x),
           (v1.y * v2.x) - (v1.x * v2.y) };
}


template <typename T>
Vector3<T> cross(const Vector3<T>& v1, const Normal3<T>& v2) {
  assert(not v1.has_nans() and not v2.has_nans());
  return { (v1.z * v2.y) - (v1.y * v2.z),
           (v1.x * v2.z) - (v1.z * v2.x),
           (v1.y * v2.x) - (v1.x * v2.y) };
}

template <typename T>
Vector3<T> cross(const Normal3<T>& v1, const Vector3<T>& v2) {
  assert(not v1.has_nans() and not v2.has_nans());
  return { (v1.z * v2.y) - (v1.y * v2.z),
           (v1.x * v2.z) - (v1.z * v2.x),
           (v1.y * v2.x) - (v1.x * v2.y) };
}

template <typename T>
T length_squared(const Vector3<T>& v) {
  return dot(v, v);
}

template <typename T>
T length(const Vector3<T>& v) {
  return std::sqrt(length_squared(v));
}

template <typename T>
Vector3<T> normalize(const Vector3<T>& v) {
  return v / length(v);
}
template <typename T>
T min_component(const Vector3<T>& v) {
  return std::min(v.x, std::min(v.y, v.z));
}

template <typename T>
T max_component(const Vector3<T>& v) {
  return std::max(v.x, std::max(v.y, v.z));
}

template <typename T>
int max_dimension(const Vector3<T>& v) {
  // return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) : ((v.y > v.z) ? 1 : 2);
  if (v.x > v.y) {
    return (v.x > v.z) ? 0 : 2;
  }
  return (v.y > v.z) ? 1 : 2;
}

template <typename T>
Vector3<T> min(const Vector3<T>& p1, const Vector3<T>& p2) {
  return Vector3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z));
}

template <typename T>
Vector3<T> max(const Vector3<T>& p1, const Vector3<T>& p2) {
  return Vector3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z));
}

template <typename T>
Vector3<T> permute(const Vector3<T>& v, int x, int y, int z) {
  return Vector3<T>(v[x], v[y], v[z]);
}

template <typename T>
Point3<T> min(const Point3<T>& p1, const Point3<T>& p2) {
  return Point3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z));
}

template <typename T>
Point3<T> max(const Point3<T>& p1, const Point3<T>& p2) {
  return Point3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z));
}

template <typename T>
Point3<T> permute(const Point3<T>& v, int x, int y, int z) {
  return Point3<T>(v[x], v[y], v[z]);
}

template <typename T, typename U>
Vector2<T> operator*(U f, const Vector2<T>& v) {
  return v * f;
}
template <typename T>
real_type dot(const Vector2<T>& v1, const Vector2<T>& v2) {
  assert(not v1.has_nans() and not v2.has_nans());
  return v1.x * v2.x + v1.y * v2.y;
}

template <typename T>
real_type abs_dot(const Vector2<T>& v1, const Vector2<T>& v2) {
  assert(not v1.has_nans() and not v2.has_nans());
  return std::abs(dot(v1, v2));
}

template <typename T>
Vector2<T> normalize(const Vector2<T>& v) {
  return v / v.length();
}
template <typename T>
Vector2<T> abs(const Vector2<T>& v) {
  return Vector2<T>(std::abs(v.x), std::abs(v.y));
}

template <typename T>
real_type distance(const Point3<T>& p1, const Point3<T>& p2) {
  return (p1 - p2).length();
}

template <typename T>
real_type distanced_squared(const Point3<T>& p1, const Point3<T>& p2) {
  return (p1 - p2).length_squared();
}

template <typename T, typename U>
Point3<T> operator*(U f, const Point3<T>& p) {
  assert(not p.has_nans());
  return p * f;
}

template <typename T>
Point3<T> lerp(real_type t, const Point3<T>& p0, const Point3<T>& p1) {
  return (1 - t) * p0 + t * p1;
}

template <typename T>
Point3<T> floor(const Point3<T>& p) {
  return Point3<T>(std::floor(p.x), std::floor(p.y), std::floor(p.z));
}

template <typename T>
Point3<T> ceil(const Point3<T>& p) {
  return Point3<T>(std::ceil(p.x), std::ceil(p.y), std::ceil(p.z));
}

template <typename T>
Point3<T> abs(const Point3<T>& p) {
  return Point3<T>(std::abs(p.x), std::abs(p.y), std::abs(p.z));
}

template <typename T>
real_type distance(const Point2<T>& p1, const Point2<T>& p2) {
  return (p1 - p2).length();
}

template <typename T>
real_type distanced_squared(const Point2<T>& p1, const Point2<T>& p2) {
  return (p1 - p2).length_squared();
}

template <typename T, typename U>
Point2<T> operator*(U f, const Point2<T>& p) {
  assert(not p.has_nans());
  return p * f;
}

template <typename T>
Point2<T> floor(const Point2<T>& p) {
  return Point2<T>(std::floor(p.x), std::floor(p.y));
}

template <typename T>
Point2<T> ceil(const Point2<T>& p) {
  return Point2<T>(std::ceil(p.x), std::ceil(p.y));
}

template <typename T>
Point2<T> lerp(real_type t, const Point2<T>& v0, const Point2<T>& v1) {
  return (1 - t) * v0 + t * v1;
}

template <typename T>
Point2<T> min(const Point2<T>& pa, const Point2<T>& pb) {
  return Point2<T>(std::min(pa.x, pb.x), std::min(pa.y, pb.y));
}

template <typename T>
Point2<T> max(const Point2<T>& pa, const Point2<T>& pb) {
  return Point2<T>(std::max(pa.x, pb.x), std::max(pa.y, pb.y));
}

template <typename T, typename U>
Normal3<T> operator*(U f, const Normal3<T>& n) {
  return Normal3<T>(f * n.x, f * n.y, f * n.z);
}

template <typename T>
Normal3<T> normalize(const Normal3<T>& n) {
  return n / n.length();
}

template <typename T>
T dot(const Normal3<T>& n1, const Vector3<T>& v2) {
  assert(not n1.has_nans() and not v2.has_nans());
  return n1.x * v2.x + n1.y * v2.y + n1.z * v2.z;
}

template <typename T>
T dot(const Vector3<T>& v1, const Normal3<T>& n2) {
  assert(not v1.has_nans() and not n2.has_nans());
  return v1.x * n2.x + v1.y * n2.y + v1.z * n2.z;
}

template <typename T>
T dot(const Normal3<T>& n1, const Normal3<T>& n2) {
  assert(not n1.has_nans() and not n2.has_nans());
  return n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
}

template <typename T>
T abs_dot(const Normal3<T>& n1, const Vector3<T>& v2) {
  assert(not n1.has_nans() and not v2.has_nans());
  return std::abs(n1.x * v2.x + n1.y * v2.y + n1.z * v2.z);
}

template <typename T>
T abs_dot(const Vector3<T>& v1, const Normal3<T>& n2) {
  assert(not v1.has_nans() and not n2.has_nans());
  return std::abs(v1.x * n2.x + v1.y * n2.y + v1.z * n2.z);
}

template <typename T>
T abs_dot(const Normal3<T>& n1, const Normal3<T>& n2) {
  assert(not n1.has_nans() and not n2.has_nans());
  return std::abs(n1.x * n2.x + n1.y * n2.y + n1.z * n2.z);
}

template <typename T>
Normal3<T> abs(const Normal3<T>& v) {
  return Normal3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

// == Ray class
template <typename T>
class Ray {
public:
  Point3<T> o;  //!< origin
  Vector3<T> d; //!< direction
  mutable T t_min, t_max; //!< parameters

  Ray() : t_min{0}, t_max{std::numeric_limits<T>::infinity()} {}
  Ray(const Point3<T>& origin, const Vector3<T>& direction,
      T start = 0, T end = std::numeric_limits<T>::infinity())
      : o{origin}, d{direction}, t_min{start}, t_max{end} {}

  Point3<T> operator()(T t) const { return o + d * t; }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Ray<T>& r) {
  os << "Ray: origin=" << r.o << ", direction=" << r.d
     << ", t_min=" << r.t_min << ", t_max=" << r.t_max;
  return os;
}

using Rayf = Ray<real_type>;

template <typename T, typename U, typename V>
T clamp(T value, U low, V high) {
  if (value < low) {
    return low;
  }
  if (value > high) {
    return high;
  }
  return value;
}

static const RGBColor color_black{0.0f, 0.0f, 0.0f};
static const RGBColor color_white{1.0f, 1.0f, 1.0f};

} // namespace ryt

#endif