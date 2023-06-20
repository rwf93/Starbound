#ifndef STAR_QUATERNION_HPP
#define STAR_QUATERNION_HPP

#include "StarVector3.hpp"
#include "StarMatrix3.hpp"

namespace Star {

template <typename T1>
class Quaternion {
public:
  // Default tolerance is 100 times type epsilon
  static T1 const Tolerance;

  static Quaternion slerp(Quaternion q, Quaternion r, T1 a);
  static T1 dot(Quaternion q1, Quaternion q2);

  static Quaternion fromRotMat(Matrix3<T1> mat);
  static Quaternion fromAxisAngle(Vector3<T1> axis, T1 angle);
  static Quaternion fromAngVel(Vector3<T1> omega);

  T1 n;
  Vector3<T1> v;

  Quaternion();
  Quaternion(T1 w, T1 x, T1 y, T1 z);
  Quaternion(Vector3<T1> axis, T1 angle);

  T1* ptr();
  T1 const* ptr() const;

  template<typename T2> Quaternion(Quaternion<T2> const& q);
  template<typename T2> Quaternion& operator=(Quaternion<T2> const& q);

  template<typename T2>
  Quaternion& operator+=(Quaternion<T2> q);
  template<typename T2>
  Quaternion& operator-=(Quaternion<T2> q);

  Quaternion& operator*=(T1 s);
  Quaternion& operator/=(T1 s);

  template<typename T2>
  Quaternion operator+(Quaternion<T2> const& q) const;
  template<typename T2>
  Quaternion operator-(Quaternion<T2> const& q) const;

  template<typename T2>
  Quaternion operator*(Quaternion<T2> const& q) const;
  template<typename T2>
  Quaternion operator*(Vector3<T2> const& q) const;

  Quaternion operator*(T1 const s) const;
  Quaternion operator/(T1 const s) const;

  Quaternion operator~() const;
  Quaternion operator-() const;
  T1 magnitude() const;

  Quaternion& normalize();

  Vector3<T1> axis() const;
  T1 angle() const;

  Matrix3<T1> toRotMat() const;

  Quaternion rotate(Quaternion q2) const;
  Vector3<T1> rotate(Vector3<T1> v) const;
};

typedef Quaternion<double> QuatD;
typedef Quaternion<float> QuatF;

template<typename T>
Quaternion<T>::Quaternion() {
  n    = 1;
  v[0] = 0;
  v[1] = 0;
  v[2] = 0;
}

template<typename T>
Quaternion<T>::Quaternion(T w, T x, T y, T z) {
  n    = w;
  v[0] = x;
  v[1] = y;
  v[2] = z;
}

template<typename T1>
Quaternion<T1>::Quaternion(Vector3<T1> axis, T1 angle) {
  T1 d = axis.magnitude();
  if (d != 0) {
    T1 s = sin(0.5*angle) / d;
    v[0] = (T1) axis[0] * s;
    v[1] = (T1) axis[1] * s;
    v[2] = (T1) axis[2] * s;
    n = cos(0.5*angle);
  }
  else {
    v[0] = v[1] = v[2] = 0;
    n = 1;
  }
}

template<typename T1> template<typename T2>
Quaternion<T1>::Quaternion(Quaternion<T2> const& q) {
  n = q.n;
  v = q.v;
}

template<typename T1> template<typename T2>
Quaternion<T1>& Quaternion<T1>::operator=(Quaternion<T2> const& q) {
  n = q.n;
  v = q.v;
  return *this;
}

template<typename T>
T Quaternion<T>::magnitude() const {
  return (T) sqrt(n*n + v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}


template<typename T1> template<typename T2>
Quaternion<T1>& Quaternion<T1>::operator+=(Quaternion<T2> q) {
  n += q.n;
  v[0] += q.v[0];
  v[1] += q.v[1];
  v[2] += q.v[2];
  return *this;
}

template<typename T1> template<typename T2>
Quaternion<T1>& Quaternion<T1>::operator-=(Quaternion<T2> q) {
  n -= q.n;
  v[0] -= q.v[0];
  v[1] -= q.v[1];
  v[2] -= q.v[2];
  return *this;
}

template<typename T1>
Quaternion<T1>& Quaternion<T1>::operator*=(T1 s) {
  n *= s;
  v[0] *= s;
  v[1] *= s;
  v[2] *= s;
  return *this;
}

template<typename T1>
Quaternion<T1> Quaternion<T1>::operator/(T1 s) const {
  return Quaternion<T1>(n/s, v[0]/s, v[1]/s, v[2]/s);
}

template<typename T1>
Quaternion<T1>& Quaternion<T1>::operator/=(T1 s) {
  n /= s;
  v /= s;
  return *this;
}

template<typename T1>
Quaternion<T1> Quaternion<T1>::operator~() const {
  return Quaternion(n, -v[0], -v[1], -v[2]);
}

template<typename T1>
Quaternion<T1> Quaternion<T1>::operator-() const {
  return Quaternion(-n, -v[0], -v[1], -v[2]);
}

template<typename T1> template<typename T2>
Quaternion<T1> Quaternion<T1>::operator+(
    Quaternion<T2> const& q2) const {
  return  Quaternion<T1>( n + q2.n,
        v[0] + q2.v[0],
        v[1] + q2.v[1],
        v[2] + q2.v[2]);
}

template<typename T1> template<typename T2>
Quaternion<T1> Quaternion<T1>::operator-(
    Quaternion<T2> const& q2) const {
  return  Quaternion<T1>( n - q2.n,
        v[0] - q2.v[0],
        v[1] - q2.v[1],
        v[2] - q2.v[2]);
}

template<typename T1> template<typename T2>
Quaternion<T1> Quaternion<T1>::operator*(
    Quaternion<T2> const& q2) const {
  return  Quaternion<T1>(
    n*q2.n - v[0]*q2.v[0] - v[1]*q2.v[1] - v[2]*q2.v[2],
    n*q2.v[0] + v[0]*q2.n + v[1]*q2.v[2] - v[2]*q2.v[1],
    n*q2.v[1] + v[1]*q2.n + v[2]*q2.v[0] - v[0]*q2.v[2],
    n*q2.v[2] + v[2]*q2.n + v[0]*q2.v[1] - v[1]*q2.v[0]);
}

template<typename T1>
Quaternion<T1> Quaternion<T1>::operator*(T1 s) const {
  return  Quaternion<T1>( n * s, v[0] * s,
        v[1] * s, v[2] * s );
}

template<typename T1> template<typename T2>
Quaternion<T1> Quaternion<T1>::operator*(Vector3<T2> const& v2) const {
  return  Quaternion<T1>( -(v[0]*v2[0] + v[1]*v2[1] + v[2]*v2[2]),
        n*v2[0] + v[1]*v2[2] - v[2]*v2[1],
        n*v2[1] + v[2]*v2[0] - v[0]*v2[2],
        n*v2[2] + v[0]*v2[1] - v[1]*v2[0]);
}

template<typename T1>
T1 Quaternion<T1>::angle() const {
  return T1(2*acos(n));
}

template<typename T1>
Vector3<T1> Quaternion<T1>::axis() const {
  T1 m = v.magnitude();

  if (m <= Tolerance)
    return Vector3<T1();
  else
    return v/m;
}

template<typename T1>
Quaternion<T1> Quaternion<T1>::rotate(Quaternion q2) const {
  return (*this)*q2*(~(*this));
}

template<typename T1>
Vector3<T1> Quaternion<T1>::rotate(Vector3<T1> v) const {
  Quaternion<T1> t;
  t = (*this)*v*(~(*this));

  return t.v;
}

template<typename T>
Quaternion<T> Quaternion<T>::slerp(Quaternion<T> q, Quaternion<T> r, T a) {
  q.normalize();
  r.normalize();
  T u, v, angle, s;
  T dot = qdot(q, r);
  if(dot < 0.0f) {
    q = -q;
    dot = qdot(q, r);
  }

  // acos gives NaN for dot slightly out of range
  if(dot > -(1.0 - Tolerance)) {
    if(dot < (1.0 - Tolerance)) {
      angle = acos(dot);
      s = sin(angle);
    } else {
      angle = 0;
      s = 0;
    }
  } else {
    angle = T(Constants::pi);
    s = 0;
  }
  if(s == 0)
    return angle<Constants::pi/2 ? q : r;

  u = sin((1-a)*angle)/s;
  v = sin(a*angle)/s;
  return norm(u*q + v*r);
}

template<typename T>
Quaternion<T> Quaternion<T>::fromRotMat(Matrix3<T> mat) {
  // quicker, but still wrong in one case
  /*
  Quaternion<T> q(1.0f, 0.0f, 0.0f, 0.0f);
  T t;
  if((t=0.25f*(1.0f + mat.r1[0] + mat.r2[1] + mat.r3[2])) > Tolerance) {
    q.n = sqrt(t);
    t = 4.0f*q.n;
    q.v[0] = (mat.r2[2]-mat.r3[1]) / t;
    q.v[1] = (mat.r3[0]-mat.r1[2]) / t;
    q.v[2] = (mat.r1[1]-mat.r2[0]) / t;
  } else if((t=-0.5f*(mat.r2[1]+mat.r3[2])) > Tolerance) {
    q.v[0] = sqrt(t);
    t=2.0f*q.v[0];
    q.v[1] = mat.r1[1]/t;
    q.v[2] = mat.r1[2]/t;
  } else if((t=0.5f*(1-mat.r3[2])) > Tolerance) {
    q.v[1]=sqrt(t);
    q.v[2]=mat.r2[2]/(2.0f*q.v[1]);
  }
  return norm(q);
  */

  Quaternion<T> quat;
  T s;
  T tr = mat[0][0] + mat[1][1] + mat[2][2];
  if(tr >= 0.0f) {
    s = sqrt(tr + 1.0f);
    quat.n = s * 0.5f;
    s = T(0.5) / s;
    quat.v[0] = (mat.r3[1] - mat.r2[2]) * s;
    quat.v[1] = (mat.r1[2] - mat.r3[0]) * s;
    quat.v[2] = (mat.r2[0] - mat.r1[1]) * s;
  }
  else {
    int i = 0;
    if(mat.r2[1] > mat.r1[0]) i = 1;
    if(mat.r3[2] > mat[i][i]) i = 2;
    switch(i) {
    case 0:
      s = sqrt(1.0f + mat.r1[0] - mat.r2[1] - mat.r3[2]);

      quat.v[0] = s*0.5f;
      s = 0.5f / s;
      quat.v[1] = (mat.r2[0] + mat.r1[1]) * s;
      quat.v[2] = (mat.r1[2] + mat.r3[0]) * s;
      quat.n = (mat.r3[1] - mat.r2[2]) * s;
      break;
    case 1:
      s = sqrt(1.0f + mat.r2[1] - mat.r1[0] - mat.r3[2]);

      quat.v[1] = s*0.5f;
      s = 0.5f / s;
      quat.v[2] = (mat.r3[1] + mat.r2[2]) * s;
      quat.v[0] = (mat.r2[0] + mat.r1[1]) * s;
      quat.n = (mat.r1[2] - mat.r3[0]) * s;
      break;
    case 2:
      s = sqrt(1.0f + mat.r3[2] - mat.r1[0] - mat.r2[1]);

      quat.v[2] = s*0.5f;
      s = 0.5f / s;
      quat.v[0] = (mat.r1[2] + mat.r3[0]) * s;
      quat.v[1] = (mat.r3[1] + mat.r2[2]) * s;
      quat.n = (mat.r2[0] - mat.r1[1]) * s;
      break;
    }
  }
  return norm(quat);
}

template<typename T>
Matrix3<T> Quaternion<T>::toRotMat() const {
  Matrix3<T> mat;

  T twx = 2*n*v[0];
  T twy = 2*n*v[1];
  T twz = 2*n*v[2];
  T txy = 2*v[0]*v[1];
  T txz = 2*v[0]*v[2];
  T tyz = 2*v[1]*v[2];

  T tx2 = 2*v[0]*v[0];
  T ty2 = 2*v[1]*v[1];
  T tz2 = 2*v[2]*v[2];


  mat.r1[0] = 1 - ty2 - tz2;
  mat.r2[0] = txy + twz;
  mat.r3[0] = txz - twy;
  mat.r1[1] = txy - twz;
  mat.r2[1] = 1 - tx2 - tz2;
  mat.r3[1] = tyz + twx;
  mat.r1[2] = txz + twy;
  mat.r2[2] = tyz - twx;
  mat.r3[2] = 1 - tx2 - ty2;
  return mat;
}

template<typename T>
Quaternion<T> Quaternion<T>::fromAxisAngle(Vector3<T> axis, T angle) {
  Quaternion<T> ret;
  ret.n = cos(angle / 2);
  ret.v = norm(axis) * sin(angle / 2);
  return ret;
}

template<typename T>
Quaternion<T> Quaternion<T>::fromAngVel(Vector3<T> axis) {
  T mag = mag(axis);
  return fromAxisAngle(axis/mag, mag);
}

template<typename T>
T Quaternion<T>::dot(Quaternion<T> q, Quaternion<T> r) {
  return q.n*r.n + (q.v * r.v);
}

template<typename T>
Quaternion<T>& Quaternion<T>::normalize() {
  operator/=(magnitude());
  return *this;
}

template<typename T>
T qdot(Quaternion<T> const& q, Quaternion<T> const& r) {
  return Quaternion<T>::dot(q, r);
}

template<typename T>
T mag(Quaternion<T> const& q) {
  return q.magnitude();
}

template<typename T>
Quaternion<T> norm(Quaternion<T> q) {
  return q.normalize();
}

template<typename T>
Vector3<T> qv_rotate(Quaternion<T> q, Vector3<T> v) {
  return q.rotate(v);
}

template<typename T>
Quaternion<T> q_rotate(Quaternion<T> q1, Quaternion<T> q2) {
  return q1.rotate(q2);
}

template<typename T1, typename T2>
Quaternion<T1> operator*(Vector3<T1> const& v, Quaternion<T2> const& q) {
  return q * v;
}

template<typename T1>
Quaternion<T1> operator*(T1 s, Quaternion<T1> const& q) {
  return q * s;
}

template<typename T1>
T1 const* Quaternion<T1>::ptr() const {
    return &n;
}

template<typename T1>
T1* Quaternion<T1>::ptr() {
    return &n;
}

}

#endif
