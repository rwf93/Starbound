#ifndef STAR_MATRIX4_HPP
#define STAR_MATRIX4_HPP

#include "StarMatrix3.hpp"
#include "StarVector4.hpp"

namespace Star {

template<typename T1> class Matrix4 {
public:
	static Matrix4<T1> identity();

	static Matrix4<T1> rotAroundPoint(Matrix3<T1> rot, Vector3<T1> point);
	static Matrix4<T1> frustrum(T1 l, T1 r, T1 b, T1 t, T1 n, T1 f);
	static Matrix4<T1> frustrumInv(T1 l, T1 r, T1 b, T1 t, T1 n, T1 f);

public:
	typedef Vector4<T1> RowT;
	RowT r1, r2, r3, r4;

	Matrix4();
	Matrix4(T1 r1c1, T1 r1c2, T1 r1c3, T1 r1c4,
			T1 r2c1, T1 r2c2, T1 r2c3, T1 r2c4,
			T1 r3c1, T1 r3c2, T1 r3c3, T1 r3c4,
			T1 r4c1, T1 r4c2, T1 r4c3, T1 r4c4 );
	Matrix4(RowT const& r1, RowT const& r2,
      RowT const& r3, RowT const& r4);

	template<typename T2> Matrix4(Matrix4<T2> const& m);
	template<typename T2> explicit Matrix4(Matrix3<T2> const& m);

	Matrix3<T1> mat3() const;
	
	T1* ptr();
	T1 const* ptr() const;

	// Copy to an existing array
	void copy(T1 *loc) const;

	Vector4<T1>& operator[](size_t const i);
	Vector4<T1> const& operator[](size_t const i) const;

	Vector4<T1> const& row(size_t r) const;
	template<typename T2>
	void setRow(unsigned int i, Vector4<T2> const& v);

	Vector4<T1> col(size_t c) const;
	template<typename T2>
	void setCol(unsigned int i, Vector4<T2> const& v);

	template<typename T2> Matrix4& operator+=(Matrix4<T2> const& m);
	template<typename T2> Matrix4& operator-=(Matrix4<T2> const& m);
	
	T1 det();
	Matrix4& transpose();
	Matrix4& opengl();
	Matrix4& inverse();
	Vector4<T1> trace() const;

	Matrix4& operator*=(T1 const& s);
	Matrix4& operator/=(T1 const& s);
	Matrix4 operator*(T1 const& s);
	Matrix4 operator/(T1 const& s);

	template<typename T2>
	Matrix4 operator+(Matrix4<T2> const& m2) const;
	template<typename T2>
	Matrix4 operator-(Matrix4<T2> const& m2) const;
	template<typename T2>
	Matrix4 operator*(Matrix4<T2> const& m2) const;

	template<typename T2>
	Vector4<T1> operator*(Vector4<T2> const& v) const;
};

typedef Matrix4<float> Mat4F;
typedef Matrix4<double> Mat4D;

template<typename T>
std::ostream& operator<<(std::ostream &os, Matrix4<T> mat) {
	os << mat[0] << std::endl;
	os << mat[1] << std::endl;
	os << mat[2] << std::endl;
	os << mat[3];
	return os;
}

template<typename T1>
Matrix4<T1> Matrix4<T1>::identity() {
	return Matrix4(1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1);
}

template<typename T1>
Vector4<T1> Matrix4<T1>::trace() const {
	return Vector4<T1>(r1[0], r2[1], r3[2], r4[3]);
}

template<typename T>
inline	Matrix4<T>::Matrix4() {}

template<typename T>
Matrix4<T>::Matrix4(
    T r1c1, T r1c2, T r1c3, T r1c4,
    T r2c1, T r2c2, T r2c3, T r2c4,
    T r3c1, T r3c2, T r3c3, T r3c4,
    T r4c1, T r4c2, T r4c3, T r4c4 ) {
	r1 = Vector4<T>(r1c1, r1c2, r1c3, r1c4);
	r2 = Vector4<T>(r2c1, r2c2, r2c3, r2c4);
	r3 = Vector4<T>(r3c1, r3c2, r3c3, r3c4);
	r4 = Vector4<T>(r4c1, r4c2, r4c3, r4c4);
}

template<typename T>
Matrix4<T>::Matrix4(const RowT& pr1, const RowT& pr2,
		const RowT& pr3, const RowT& pr4) {
	r1 = pr1;
	r2 = pr2;
	r3 = pr3;
	r4 = pr4;
}

template<typename T1>
Vector4<T1>& Matrix4<T1>::operator[](const size_t i) {
	return *(&r1 + i);
}

template<typename T1>
const Vector4<T1>& Matrix4<T1>::operator[](const size_t i) const {
	return *(&r1 + i);
}

template<typename T1>
T1* Matrix4<T1>::ptr() {
	return &r1[0];
}

template<typename T1>
const T1* Matrix4<T1>::ptr() const {
	return &r1[0];
}


template<typename T1>
void Matrix4<T1>::copy(T1* loc) const {
	r1.copy(loc);
	r2.copy(loc + 4);
	r3.copy(loc + 8);
	r4.copy(loc + 12);
}

template<typename T1>
template<typename T2> Matrix4<T1>::Matrix4(const Matrix3<T2> &m) {
	r1 = Vector4<T1>(m[0], 0);
	r2 = Vector4<T1>(m[1], 0);
	r3 = Vector4<T1>(m[2], 0);
	r4 = Vector4<T1>(T1(0), T1(0), T1(0), T1(1));
}

template<typename T1>
Matrix3<T1> Matrix4<T1>::mat3() const {
	return Matrix3<T1>(r1, r2, r3);
}

template<typename T1>
const Vector4<T1>& Matrix4<T1>::row(size_t i) const {
	return operator[](i);
}

template<typename T1>
Vector4<T1> Matrix4<T1>::col(size_t c) const {
	return Vector4<T1>(r1[c], r2[c], r3[c], r4[c]);
}

template<typename T1> template<typename T2>
void Matrix4<T1>::setRow(unsigned int c, const Vector4<T2> &r) {
	operator[](c) = r;
}

template<typename T1> template<typename T2>
void Matrix4<T1>::setCol(unsigned int c, const Vector4<T2> &v) {
	r1[c] = v[0];
	r2[c] = v[1];
	r3[c] = v[2];
	r4[c] = v[3];
}

template<typename T1> template<typename T2>
Matrix4<T1>::Matrix4(const Matrix4<T2> &m) {
	r1 = m[0];
	r2 = m[1];
	r3 = m[2];
	r4 = m[3];
}
	
template<typename T1> template<typename T2>
Matrix4<T1>& Matrix4<T1>::operator+=(const Matrix4<T2> &m) {
	r1 += m[0];
	r2 += m[1];
	r3 += m[2];
	r4 += m[3];
}

template<typename T1> template<typename T2>
Matrix4<T1>& Matrix4<T1>::operator-=(const Matrix4<T2> &m) {
	r1 -= m[0];
	r2 -= m[1];
	r3 -= m[2];
	r4 -= m[3];
}

template<typename T>
Matrix4<T>& Matrix4<T>::transpose() {
	return operator=(Matrix4(	r1[0], r2[0], r3[0], r4[0],
					r1[1], r2[1], r3[1], r4[1],
					r1[2], r2[2], r3[2], r4[2],
					r1[3], r2[3], r3[3], r4[3]));
}

template<typename T>
Matrix4<T>& Matrix4<T>::opengl() {
	return transpose();
}

template<typename T>
Matrix4<T>& Matrix4<T>::operator*=(const T &s) {
	r1 *= s;
	r2 *= s;
	r3 *= s;
	r4 *= s;
	return *this;
}

template<typename T>
Matrix4<T>& Matrix4<T>::operator/=(const T &s) {
	r1 /= s;
	r2 /= s;
	r3 /= s;
	r4 /= s;
	return *this;
}

template <typename T1> template<typename T2>
Matrix4<T1> Matrix4<T1>::operator+(const Matrix4<T2> &m2) const {
	return Matrix4<T1>(	r1 + m2[0],
				r2 + m2[1],
				r3 + m2[2],
				r4 + m2[3]);
}

template <typename T1> template<typename T2>
Matrix4<T1> Matrix4<T1>::operator-(const Matrix4<T2> &m2) const {
	return Matrix4<T1>(	r1 - m2[0],
				r2 - m2[1],
				r3 - m2[2],
				r4 - m2[3]);
}


template <typename T1> template<typename T2>
Matrix4<T1> Matrix4<T1>::operator*(const Matrix4<T2> &m2) const {
	return Matrix4<T1>(
		row(0)*m2.col(0), row(0)*m2.col(1),
		row(0)*m2.col(2), row(0)*m2.col(3),
		row(1)*m2.col(0), row(1)*m2.col(1),
		row(1)*m2.col(2), row(1)*m2.col(3),
		row(2)*m2.col(0), row(2)*m2.col(1),
		row(2)*m2.col(2), row(2)*m2.col(3),
		row(3)*m2.col(0), row(3)*m2.col(1),
		row(3)*m2.col(2), row(3)*m2.col(3));
}

template<typename T1>
Matrix4<T1> Matrix4<T1>::operator*(const T1 &s) {
	return Matrix4(r1*s, r2*s, r3*s, r4*s);
}

template<typename T1>
Matrix4<T1> Matrix4<T1>::operator/(const T1 &s) {
	return Matrix4<T1>(r1/s, r2/s, r3/s, r4/s);
}

template <typename T1> template<typename T2>
Vector4<T1> Matrix4<T1>::operator*(const Vector4<T2>& v) const {
	return Vector4<T1>(r1*v, r2*v, r3*v, r4*v);
}

template <typename T1>
Matrix4<T1> Matrix4<T1>::rotAroundPoint(Matrix3<T1> rot,
		Vector3<T1> point) {
	Matrix4<T1> final(rot);
	final[0][3] =
		rot[0][0]*point[0] + rot[0][1]*point[1] + rot[0][0]*point[2]-point[0];
	final[1][3] =
		rot[1][0]*point[0] + rot[1][1]*point[1] + rot[1][2]*point[2]-point[1];
	final[2][3] =
		rot[2][0]*point[0] + rot[2][1]*point[1] + rot[2][2]*point[2]-point[2];
	return final;
}

template<typename T>
Matrix4<T> Matrix4<T>::frustrumInv(T l, T r, T b, T t, T n, T f) {
	return Matrix4<T>(
		(r-l)/(2*n), 0, 0, (r+l)/(2*n),
		0, (t-b)/(2*n), 0, (t+b)/(2*n),
		0, 0, 0, -1,
		0, 0, -(f-n)/(2*f*n), (f+n)/(2*f*n));
}

template<typename T>
Matrix4<T> Matrix4<T>::frustrum(T l, T r, T b, T t, T n, T f) {
	return Matrix4<T>(
		2*n/(r-l), 0, 0, (r+l)/(r-l),
		0, 2/(t-b), 0, (t+b)/(t-b),
		0, 0, -2/(f-n), (f+n)/(f-n),
		0, 0, 0, 1);
}

template<typename T>
Matrix4<T> transpose(Matrix4<T> m) {
	return m.transpose();
}

template<typename T>
Matrix4<T> opengl(Matrix4<T> m) {
	return m.opengl();
}

template<typename T>
Matrix4<T> operator*(const T &s, const Matrix4<T> &m) {
	return m * s;
}

}

#endif
