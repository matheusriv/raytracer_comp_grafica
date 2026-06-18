#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "geometry.hpp"
#include <cstring>
#include <algorithm>
#include <cmath>

namespace ryt {

template <typename T>
class Matrix4x4 {
public:
    T m[4][4];
    Matrix4x4() {
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
        m[0][1] = m[0][2] = m[0][3] = 0;
        m[1][0] = m[1][2] = m[1][3] = 0;
        m[2][0] = m[2][1] = m[2][3] = 0;
        m[3][0] = m[3][1] = m[3][2] = 0;
    }
    Matrix4x4(const T mat[4][4]) { memcpy(m, mat, 16 * sizeof(T)); }
    Matrix4x4(T t00, T t01, T t02, T t03,
              T t10, T t11, T t12, T t13,
              T t20, T t21, T t22, T t23,
              T t30, T t31, T t32, T t33) {
        m[0][0] = t00; m[0][1] = t01; m[0][2] = t02; m[0][3] = t03;
        m[1][0] = t10; m[1][1] = t11; m[1][2] = t12; m[1][3] = t13;
        m[2][0] = t20; m[2][1] = t21; m[2][2] = t22; m[2][3] = t23;
        m[3][0] = t30; m[3][1] = t31; m[3][2] = t32; m[3][3] = t33;
    }
   
    bool operator==(const Matrix4x4 &m2) const {
        for(int i=0;i<4;++i)
            for(int j=0;j<4;++j)
                if(m[i][j] != m2.m[i][j]) return false;
        return true;
    }
    bool operator!=(const Matrix4x4 &m2) const { return !(*this == m2); }
   
    Matrix4x4 operator*(const Matrix4x4 &m2) const {
        Matrix4x4 r;
        for(int i=0;i<4;++i)
            for(int j=0;j<4;++j)
                r.m[i][j] = m[i][0] * m2.m[0][j] + m[i][1] * m2.m[1][j] +
                            m[i][2] * m2.m[2][j] + m[i][3] * m2.m[3][j];
        return r;
    }
   
    Matrix4x4 transpose() const {
        return Matrix4x4(m[0][0], m[1][0], m[2][0], m[3][0],
                         m[0][1], m[1][1], m[2][1], m[3][1],
                         m[0][2], m[1][2], m[2][2], m[3][2],
                         m[0][3], m[1][3], m[2][3], m[3][3]);
    }
   
    Matrix4x4 inverse() const {
        int indxc[4], indxr[4];
        int ipiv[4] = {0, 0, 0, 0};
        T minv[4][4];
        memcpy(minv, m, 4 * 4 * sizeof(T));
        for (int i = 0; i < 4; i++) {
            int irow = 0, icol = 0;
            T big = 0;
            for (int j = 0; j < 4; j++) {
                if (ipiv[j] != 1) {
                    for (int k = 0; k < 4; k++) {
                        if (ipiv[k] == 0) {
                            if (std::abs(minv[j][k]) >= big) {
                                big = std::abs(minv[j][k]);
                                irow = j;
                                icol = k;
                            }
                        } else if (ipiv[k] > 1) {
                            return Matrix4x4(); // Singular matrix
                        }
                    }
                }
            }
            ++ipiv[icol];
            if (irow != icol) {
                for (int k = 0; k < 4; ++k) std::swap(minv[irow][k], minv[icol][k]);
            }
            indxr[i] = irow;
            indxc[i] = icol;
            if (minv[icol][icol] == 0) return Matrix4x4();
            T pivinv = 1.0 / minv[icol][icol];
            minv[icol][icol] = 1.0;
            for (int j = 0; j < 4; j++) minv[icol][j] *= pivinv;
            for (int j = 0; j < 4; j++) {
                if (j != icol) {
                    T save = minv[j][icol];
                    minv[j][icol] = 0;
                    for (int k = 0; k < 4; k++) minv[j][k] -= minv[icol][k] * save;
                }
            }
        }
        for (int j = 3; j >= 0; j--) {
            if (indxr[j] != indxc[j]) {
                for (int k = 0; k < 4; k++) std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
            }
        }
        return Matrix4x4(minv);
    }
};

class Transform {
private:
    Matrix4x4<real_type> m, mInv;
public:
    Transform() {}
    Transform(const real_type mat[4][4]) {
        m = Matrix4x4<real_type>(mat);
        mInv = m.inverse();
    }
    Transform(const Matrix4x4<real_type> &m) : m(m), mInv(m.inverse()) {}
    Transform(const Matrix4x4<real_type> &m, const Matrix4x4<real_type> &mInv) : m(m), mInv(mInv) {}
   
    const Matrix4x4<real_type>& get_matrix() const { return m; }
    const Matrix4x4<real_type>& get_inverse_matrix() const { return mInv; }
   
    Transform inverse() const {
        return Transform(mInv, m);
    }
   
    Transform transpose() const {
        return Transform(m.transpose(), mInv.transpose());
    }
   
    bool operator==(const Transform &t) const { return m == t.m; }
    bool operator!=(const Transform &t) const { return m != t.m; }
   
    template <typename T> inline Point3<T> operator()(const Point3<T> &p) const {
        T x = p.x, y = p.y, z = p.z;
        T xp = m.m[0][0]*x + m.m[0][1]*y + m.m[0][2]*z + m.m[0][3];
        T yp = m.m[1][0]*x + m.m[1][1]*y + m.m[1][2]*z + m.m[1][3];
        T zp = m.m[2][0]*x + m.m[2][1]*y + m.m[2][2]*z + m.m[2][3];
        T wp = m.m[3][0]*x + m.m[3][1]*y + m.m[3][2]*z + m.m[3][3];
        if (wp == 1) return Point3<T>(xp, yp, zp);
        return Point3<T>(xp/wp, yp/wp, zp/wp);
    }
   
    template <typename T> inline Vector3<T> operator()(const Vector3<T> &v) const {
        T x = v.x, y = v.y, z = v.z;
        return Vector3<T>(m.m[0][0]*x + m.m[0][1]*y + m.m[0][2]*z,
                          m.m[1][0]*x + m.m[1][1]*y + m.m[1][2]*z,
                          m.m[2][0]*x + m.m[2][1]*y + m.m[2][2]*z);
    }
   
    template <typename T> inline Normal3<T> operator()(const Normal3<T> &n) const {
        T x = n.x, y = n.y, z = n.z;
        return Normal3<T>(mInv.m[0][0]*x + mInv.m[1][0]*y + mInv.m[2][0]*z,
                          mInv.m[0][1]*x + mInv.m[1][1]*y + mInv.m[2][1]*z,
                          mInv.m[0][2]*x + mInv.m[1][2]*y + mInv.m[2][2]*z);
    }
   
    template <typename T> inline Ray<T> operator()(const Ray<T> &r) const {
        Ray<T> ret = r;
        ret.o = (*this)(r.o);
        ret.d = (*this)(r.d);
        return ret;
    }
   
    template <typename T> inline Bounds3<T> operator()(const Bounds3<T> &b) const {
        const Transform &M = *this;
        Bounds3<T> ret;
        ret = Union(ret, Bounds3<T>(M(Point3<T>(b.p_min.x, b.p_min.y, b.p_min.z)), M(Point3<T>(b.p_max.x, b.p_min.y, b.p_min.z))));
        ret = Union(ret, Bounds3<T>(M(Point3<T>(b.p_min.x, b.p_max.y, b.p_min.z)), M(Point3<T>(b.p_max.x, b.p_max.y, b.p_min.z))));
        ret = Union(ret, Bounds3<T>(M(Point3<T>(b.p_min.x, b.p_min.y, b.p_max.z)), M(Point3<T>(b.p_max.x, b.p_min.y, b.p_max.z))));
        ret = Union(ret, Bounds3<T>(M(Point3<T>(b.p_min.x, b.p_max.y, b.p_max.z)), M(Point3<T>(b.p_max.x, b.p_max.y, b.p_max.z))));
        return ret;
    }
   
    Transform operator*(const Transform &t2) const {
        return Transform(m * t2.m, t2.mInv * mInv);
    }
};

inline Transform translate(const Vector3f &delta) {
    Matrix4x4<real_type> m(1, 0, 0, delta.x, 0, 1, 0, delta.y, 0, 0, 1, delta.z, 0, 0, 0, 1);
    Matrix4x4<real_type> minv(1, 0, 0, -delta.x, 0, 1, 0, -delta.y, 0, 0, 1, -delta.z, 0, 0, 0, 1);
    return Transform(m, minv);
}

inline Transform scale(real_type x, real_type y, real_type z) {
    Matrix4x4<real_type> m(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
    Matrix4x4<real_type> minv(1.0/x, 0, 0, 0, 0, 1.0/y, 0, 0, 0, 0, 1.0/z, 0, 0, 0, 0, 1);
    return Transform(m, minv);
}

inline Transform rotate(real_type theta, const Vector3f &axis) {
    Vector3f a = normalize(axis);
    real_type sinTheta = std::sin(theta * M_PI / 180.0f);
    real_type cosTheta = std::cos(theta * M_PI / 180.0f);
    Matrix4x4<real_type> m;
    m.m[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
    m.m[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
    m.m[0][2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
    m.m[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
    m.m[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
    m.m[1][2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
    m.m[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
    m.m[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
    m.m[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
    m.m[3][3] = 1;
    return Transform(m, m.transpose());
}

} // namespace ryt
#endif // TRANSFORM_HPP