#pragma once
#include <cmath>  // 包含sqrtf、max等数学函数
#include <iostream>
#include <filesystem>
#include <string_view>
//#define NOMINMAX
#undef min
#undef max
#define SQ(x) ((x) * (x))
#define PI       3.14159265358979323846
#define WIDTH 1280
#define HEIGHT 720

template<typename T>
static T clamp(const T value, const T minValue, const T maxValue)
{
	return std::max(std::min(value, maxValue), minValue);
}

class Vec3
{
public:
	union {
		struct {
			float x;
			float y;
			float z;
		};
		float coords[3];
	};
	Vec3() { x = 0; y = 0; z = 0; }
	Vec3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
	Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
	Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
	Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
	Vec3 operator/(const Vec3& v) const { return Vec3(x / v.x, y / v.y, z / v.z); }
	Vec3& operator+=(const Vec3& v) { x += v.x, y += v.y, z += v.z; return *this; }
	Vec3& operator-=(const Vec3& v) { x -= v.x, y -= v.y, z -= v.z; return *this; }
	Vec3& operator*=(const Vec3& v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
	Vec3& operator/=(const Vec3& v) { x /= v.x, y /= v.y, z /= v.z; return *this; }
	Vec3 operator*(const float v) const { return Vec3(x * v, y * v, z * v); }
	Vec3 operator/(const float v) const { float iv = 1.0f / v; return Vec3(x * iv, y * iv, z * iv); }
	Vec3& operator*=(const float v) { x *= v, y *= v, z *= v; return *this; }
	Vec3& operator/=(const float v) { float iv = 1.0f / v; x *= iv, y *= iv, z *= iv; return *this; }
	Vec3 operator-() const { return Vec3(-x, -y, -z); }
	float length() const { return sqrtf(SQ(x) + SQ(y) + SQ(z)); }
	float lengthSq() const { return (SQ(x) + SQ(y) + SQ(z)); }
	Vec3 normalize() const { float l = 1.0f / sqrtf(SQ(x) + SQ(y) + SQ(z)); return Vec3(x * l, y * l, z * l); }
	float normalize_getLength() { float l = sqrtf(SQ(x) + SQ(y) + SQ(z)); float il = 1.0f / l; x *= il; y *= il; z *= il; return l; }
};

static float Dot(const Vec3& v1, const Vec3& v2) { return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z)); }
static Vec3 Cross(const Vec3& v1, const Vec3& v2) { return Vec3((v1.y * v2.z) - (v1.z * v2.y), (v1.z * v2.x) - (v1.x * v2.z), (v1.x * v2.y) - (v1.y * v2.x)); }
static Vec3 Max(const Vec3& v1, const Vec3& v2) { return Vec3(std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z)); }
static Vec3 Min(const Vec3& v1, const Vec3& v2) { return Vec3(std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z)); }


class Vec4
{
public:
	float x, y, z, w;

	Vec4() : x(0), y(0), z(0), w(1) {}
	Vec4(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}

	void divW() { if (w != 0) { x /= w; y /= w; z /= w; w = 1; } }



	//Vec4(const Vec3& vec3, float _w = 1.0f)
	//	: v{ vec3.x, vec3.y, vec3.z, _w }  // 从Vec3扩展（默认w=1，适合齐次坐标点）
	//{
	//}

	// 加法运算
	Vec4 operator+(const Vec4& pVec) const
	{
		return Vec4(x + pVec.x, y + pVec.y, z + pVec.z, w + pVec.w);
	}

	Vec4& operator+=(const Vec4& pVec)
	{
		x += pVec.x;
		y += pVec.y;
		z += pVec.z;
		w += pVec.w;
		return *this;
	}

	// 乘法运算（分量乘、标量乘）
	Vec4 operator*(const Vec4& pVec) const
	{
		return Vec4(x * pVec.x, y * pVec.y, z * pVec.z, w * pVec.w);
	}

	Vec4 operator*(const float val) const
	{
		return Vec4(x * val, y * val, z * val, w * val);
	}

	// 除法运算（分量除、标量除）
	Vec4 operator/(const Vec4& pVec) const
	{
		return Vec4(x / pVec.x, y / pVec.y, z / pVec.z, w / pVec.w);
	}

	Vec4 operator/(const float val) const
	{
		return Vec4(x / val, y / val, z / val, w / val);
	}

	Vec4& operator/=(const Vec4& pVec)
	{
		x /= pVec.x;
		y /= pVec.y;
		z /= pVec.z;
		w /= pVec.w;
		return *this;
	}

	Vec4& operator/=(const float val)
	{
		x /= val;
		y /= val;
		z /= val;
		w /= val;
		return *this;
	}

	// 负号运算（分量取反）
	Vec4 operator-() const
	{
		return Vec4(-x, -y, -z, -w);
	}
	Vec4 operator-(const Vec4& v)
	{
		return Vec4(x - v.x, y - v.y, z - v.z, w - v.w);
	}

	// 减法运算（复合赋值）
	Vec4& operator-=(const Vec4& pVec)
	{
		x -= pVec.x;
		y -= pVec.y;
		z -= pVec.z;
		w -= pVec.w;
		return *this;
	}

	// 长度平方（减少开方开销）
	float LengthSQ()
	{
		return SQ(x) + SQ(y) + SQ(z) + SQ(w);
	}

	// 长度（模长）
	float Length() const
	{
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	// 归一化（返回新向量，不修改原向量）
	

	// 归一化（修改原向量，返回原长度）
	float normalize_GetLength()
	{
		float length = sqrtf(x * x + y * y + z * z + w * w);
		float len = 1.0f / length;
		x *= len;
		y *= len;
		z *= len;
		w *= len;
		return length;
	}

	

	
	//叉积
	static Vec4 Cross(const Vec4& a, const Vec4& b)
	{
		// 3D叉乘（忽略w分量）
		return Vec4(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x,
			0.0f
		);
	}

	static float Dot(const Vec4& a, const Vec4& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	Vec4 Normalize() const
	{
		float len = Length();
		return len > 0 ? *this / len : Vec4(0, 0, 0, 0);
	}

	//void Normalize()
	//{
	//	// 点无意义
	//    if (w != 0.f)
	//    {
	//		return;
	//    }
	//	// 如果是方向
	//	float length = sqrtf(x * x + y * y + z * z + w * w);

	//	if (length == 0.f)
	//	{
	//		return;
	//	}

	//	float invLength = 1.0f / length;

	//	x*= invLength;
	//	y*= invLength;
	//	z*= invLength;
	//	w*= invLength;

	//}
};

class alignas(64) Matrix
{
public:
	union
	{
		float a[4][4];
		float m[16];
	};
	Matrix() { identity(); }
	Matrix(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
	{
		a[0][0] = m00;
		a[0][1] = m01;
		a[0][2] = m02;
		a[0][3] = m03;
		a[1][0] = m10;
		a[1][1] = m11;
		a[1][2] = m12;
		a[1][3] = m13;
		a[2][0] = m20;
		a[2][1] = m21;
		a[2][2] = m22;
		a[2][3] = m23;
		a[3][0] = m30;
		a[3][1] = m31;
		a[3][2] = m32;
		a[3][3] = m33;
	}
	void identity()
	{
		memset(m, 0, 16 * sizeof(float));
		m[0] = 1.0f;
		m[5] = 1.0f;
		m[10] = 1.0f;
		m[15] = 1.0f;
	}
	Matrix transpose()
	{
		return Matrix(a[0][0], a[1][0], a[2][0], a[3][0],
			a[0][1], a[1][1], a[2][1], a[3][1],
			a[0][2], a[1][2], a[2][2], a[3][2],
			a[0][3], a[1][3], a[2][3], a[3][3]);
	}
	float& operator[](int index)
	{
		return m[index];
	}
	static Matrix translation(const Vec3& v)
	{
		Matrix mat;
		mat.a[0][3] = v.x;
		mat.a[1][3] = v.y;
		mat.a[2][3] = v.z;
		return mat;
	}
	static Matrix scaling(const Vec3& v)
	{
		Matrix mat;
		mat.m[0] = v.x;
		mat.m[5] = v.y;
		mat.m[10] = v.z;
		return mat;
	}
	static Matrix rotateX(float theta)
	{
		Matrix mat;
		float ct = cosf(theta);
		float st = sinf(theta);
		mat.m[5] = ct;
		mat.m[6] = st;
		mat.m[9] = -st;
		mat.m[10] = ct;
		return mat;
	}
	static Matrix rotateY(float theta)
	{
		Matrix mat;
		float ct = cosf(theta);
		float st = sinf(theta);
		mat.m[0] = ct;
		mat.m[2] = -st;
		mat.m[8] = st;
		mat.m[10] = ct;
		return mat;
	}
	static Matrix rotateZ(float theta)
	{
		Matrix mat;
		float ct = cosf(theta);
		float st = sinf(theta);
		mat.m[0] = ct;
		mat.m[1] = st;
		mat.m[4] = -st;
		mat.m[5] = ct;
		return mat;
	}
	Matrix mul(const Matrix& matrix) const
	{
		Matrix ret;

		ret.m[0] = m[0] * matrix.m[0] + m[4] * matrix.m[1] + m[8] * matrix.m[2] + m[12] * matrix.m[3];
		ret.m[1] = m[1] * matrix.m[0] + m[5] * matrix.m[1] + m[9] * matrix.m[2] + m[13] * matrix.m[3];
		ret.m[2] = m[2] * matrix.m[0] + m[6] * matrix.m[1] + m[10] * matrix.m[2] + m[14] * matrix.m[3];
		ret.m[3] = m[3] * matrix.m[0] + m[7] * matrix.m[1] + m[11] * matrix.m[2] + m[15] * matrix.m[3];

		ret.m[4] = m[0] * matrix.m[4] + m[4] * matrix.m[5] + m[8] * matrix.m[6] + m[12] * matrix.m[7];
		ret.m[5] = m[1] * matrix.m[4] + m[5] * matrix.m[5] + m[9] * matrix.m[6] + m[13] * matrix.m[7];
		ret.m[6] = m[2] * matrix.m[4] + m[6] * matrix.m[5] + m[10] * matrix.m[6] + m[14] * matrix.m[7];
		ret.m[7] = m[3] * matrix.m[4] + m[7] * matrix.m[5] + m[11] * matrix.m[6] + m[15] * matrix.m[7];

		ret.m[8] = m[0] * matrix.m[8] + m[4] * matrix.m[9] + m[8] * matrix.m[10] + m[12] * matrix.m[11];
		ret.m[9] = m[1] * matrix.m[8] + m[5] * matrix.m[9] + m[9] * matrix.m[10] + m[13] * matrix.m[11];
		ret.m[10] = m[2] * matrix.m[8] + m[6] * matrix.m[9] + m[10] * matrix.m[10] + m[14] * matrix.m[11];
		ret.m[11] = m[3] * matrix.m[8] + m[7] * matrix.m[9] + m[11] * matrix.m[10] + m[15] * matrix.m[11];

		ret.m[12] = m[0] * matrix.m[12] + m[4] * matrix.m[13] + m[8] * matrix.m[14] + m[12] * matrix.m[15];
		ret.m[13] = m[1] * matrix.m[12] + m[5] * matrix.m[13] + m[9] * matrix.m[14] + m[13] * matrix.m[15];
		ret.m[14] = m[2] * matrix.m[12] + m[6] * matrix.m[13] + m[10] * matrix.m[14] + m[14] * matrix.m[15];
		ret.m[15] = m[3] * matrix.m[12] + m[7] * matrix.m[13] + m[11] * matrix.m[14] + m[15] * matrix.m[15];

		return ret;
	}
	Matrix operator*(const Matrix& matrix)
	{
		return mul(matrix);
	}
	Vec3 mulVec(const Vec3& v)
	{
		return Vec3(
			(v.x * m[0] + v.y * m[1] + v.z * m[2]),
			(v.x * m[4] + v.y * m[5] + v.z * m[6]),
			(v.x * m[8] + v.y * m[9] + v.z * m[10]));
	}
	Vec3 mulPoint(const Vec3& v)
	{
		Vec3 v1 = Vec3(
			(v.x * m[0] + v.y * m[1] + v.z * m[2]) + m[3],
			(v.x * m[4] + v.y * m[5] + v.z * m[6]) + m[7],
			(v.x * m[8] + v.y * m[9] + v.z * m[10]) + m[11]);
		float w;
		w = (m[12] * v.x) + (m[13] * v.y) + (m[14] * v.z) + m[15];
		w = 1.0f / w;
		return (v1 * w);
	}
	Matrix operator=(const Matrix& matrix)
	{
		memcpy(m, matrix.m, sizeof(float) * 16);
		return (*this);
	}
	Matrix invert() // Unrolled inverse from MESA library
	{
		Matrix inv;
		inv[0] = m[5] * m[10] * m[15] -
			m[5] * m[11] * m[14] -
			m[9] * m[6] * m[15] +
			m[9] * m[7] * m[14] +
			m[13] * m[6] * m[11] -
			m[13] * m[7] * m[10];
		inv[4] = -m[4] * m[10] * m[15] +
			m[4] * m[11] * m[14] +
			m[8] * m[6] * m[15] -
			m[8] * m[7] * m[14] -
			m[12] * m[6] * m[11] +
			m[12] * m[7] * m[10];
		inv[8] = m[4] * m[9] * m[15] -
			m[4] * m[11] * m[13] -
			m[8] * m[5] * m[15] +
			m[8] * m[7] * m[13] +
			m[12] * m[5] * m[11] -
			m[12] * m[7] * m[9];
		inv[12] = -m[4] * m[9] * m[14] +
			m[4] * m[10] * m[13] +
			m[8] * m[5] * m[14] -
			m[8] * m[6] * m[13] -
			m[12] * m[5] * m[10] +
			m[12] * m[6] * m[9];
		inv[1] = -m[1] * m[10] * m[15] +
			m[1] * m[11] * m[14] +
			m[9] * m[2] * m[15] -
			m[9] * m[3] * m[14] -
			m[13] * m[2] * m[11] +
			m[13] * m[3] * m[10];
		inv[5] = m[0] * m[10] * m[15] -
			m[0] * m[11] * m[14] -
			m[8] * m[2] * m[15] +
			m[8] * m[3] * m[14] +
			m[12] * m[2] * m[11] -
			m[12] * m[3] * m[10];
		inv[9] = -m[0] * m[9] * m[15] +
			m[0] * m[11] * m[13] +
			m[8] * m[1] * m[15] -
			m[8] * m[3] * m[13] -
			m[12] * m[1] * m[11] +
			m[12] * m[3] * m[9];
		inv[13] = m[0] * m[9] * m[14] -
			m[0] * m[10] * m[13] -
			m[8] * m[1] * m[14] +
			m[8] * m[2] * m[13] +
			m[12] * m[1] * m[10] -
			m[12] * m[2] * m[9];
		inv[2] = m[1] * m[6] * m[15] -
			m[1] * m[7] * m[14] -
			m[5] * m[2] * m[15] +
			m[5] * m[3] * m[14] +
			m[13] * m[2] * m[7] -
			m[13] * m[3] * m[6];
		inv[6] = -m[0] * m[6] * m[15] +
			m[0] * m[7] * m[14] +
			m[4] * m[2] * m[15] -
			m[4] * m[3] * m[14] -
			m[12] * m[2] * m[7] +
			m[12] * m[3] * m[6];
		inv[10] = m[0] * m[5] * m[15] -
			m[0] * m[7] * m[13] -
			m[4] * m[1] * m[15] +
			m[4] * m[3] * m[13] +
			m[12] * m[1] * m[7] -
			m[12] * m[3] * m[5];
		inv[14] = -m[0] * m[5] * m[14] +
			m[0] * m[6] * m[13] +
			m[4] * m[1] * m[14] -
			m[4] * m[2] * m[13] -
			m[12] * m[1] * m[6] +
			m[12] * m[2] * m[5];
		inv[3] = -m[1] * m[6] * m[11] +
			m[1] * m[7] * m[10] +
			m[5] * m[2] * m[11] -
			m[5] * m[3] * m[10] -
			m[9] * m[2] * m[7] +
			m[9] * m[3] * m[6];
		inv[7] = m[0] * m[6] * m[11] -
			m[0] * m[7] * m[10] -
			m[4] * m[2] * m[11] +
			m[4] * m[3] * m[10] +
			m[8] * m[2] * m[7] -
			m[8] * m[3] * m[6];
		inv[11] = -m[0] * m[5] * m[11] +
			m[0] * m[7] * m[9] +
			m[4] * m[1] * m[11] -
			m[4] * m[3] * m[9] -
			m[8] * m[1] * m[7] +
			m[8] * m[3] * m[5];
		inv[15] = m[0] * m[5] * m[10] -
			m[0] * m[6] * m[9] -
			m[4] * m[1] * m[10] +
			m[4] * m[2] * m[9] +
			m[8] * m[1] * m[6] -
			m[8] * m[2] * m[5];
		float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
		if (det == 0)
		{
			exit(0); // Perhaps add logging here. This should not happen in a games context
		}
		det = 1.0 / det;
		for (int i = 0; i < 16; i++)
		{
			inv[i] = inv[i] * det;
		}
		return inv;
	}
	static Matrix lookAt(const Vec3& from, const Vec3& to, const Vec3& up)
	{
		Matrix mat;
		Vec3 dir = (to - from).normalize();
		Vec3 left = Cross(up, dir).normalize();
		Vec3 newUp = Cross(dir, left);
		mat.a[0][0] = left.x;
		mat.a[0][1] = left.y;
		mat.a[0][2] = left.z;
		mat.a[1][0] = newUp.x;
		mat.a[1][1] = newUp.y;
		mat.a[1][2] = newUp.z;
		mat.a[2][0] = dir.x;
		mat.a[2][1] = dir.y;
		mat.a[2][2] = dir.z;
		mat.a[0][3] = -Dot(from, left);
		mat.a[1][3] = -Dot(from, newUp);
		mat.a[2][3] = -Dot(from, dir);
		mat.a[3][3] = 1;
		return mat;
	}
	static Matrix perspective(const float n, const float f, float aspect, const float fov) // FOV in degrees, outputs transposed Matrix for DX
	{
		Matrix pers;
		memset(pers.m, 0, sizeof(float) * 16);
		float t = 1.0f / (tanf(fov * 0.5f * 3.141592654f / 180.0f));
		pers.a[0][0] = t / aspect;
		pers.a[1][1] = t;
		pers.a[2][2] = f / (f - n);
		pers.a[2][3] = -(f * n) / (f - n);
		pers.a[3][2] = 1.0f;
		return pers;
	}
	static Matrix rotateAxis(const Vec3& axis, float angle)
	{
		Vec3 u = axis.normalize();
		float c = cosf(angle);
		float s = sinf(angle);
		float t = 1.0f - c;

		return Matrix(
			t * u.x * u.x + c, t * u.x * u.y + s * u.z, t * u.x * u.z - s * u.y, 0,
			t * u.x * u.y - s * u.z, t * u.y * u.y + c, t * u.y * u.z + s * u.x, 0,
			t * u.x * u.z + s * u.y, t * u.y * u.z - s * u.x, t * u.z * u.z + c, 0,
			0, 0, 0, 1
		);
	}
};

// 简化矩阵乘法运算符（可选）
//Matrix operator*(const Matrix& a, const Matrix& b) { return a.mul(b); }
//Vec4 operator*(const Matrix& a, const Vec4& b) { return a.mul(b); }
struct Colour
{
	float r, g, b;

	Colour() : r(0.0f), g(0.0f), b(0.0f) {}  // 默认构造，初始化为(0,0)
	Colour(float x_, float y_, float z_) : r(x_), g(y_), b(z_) {}  // 带参数构造，指定x和y

	// 重载乘法运算符：Color * float（分量分别乘以标量）
	Colour operator*(float scalar) const
	{
		return { r * scalar, g * scalar, b * scalar };
	}

	// 重载加法运算符：Color + Color（分量分别相加）
	Colour operator+(const Colour& other) const
	{
		return { r + other.r, g + other.g, b + other.b };
	}
	Colour operator/(const float val) const
	{
		return { r / val, g / val,b / val };
	}
	Colour operator*(const Colour col) const
	{
		return { r * col.r,g * col.g,b * col.b };
	}
};

class Quaternion
{
public:
	union {
		struct {
			float a;
			float b;
			float c;
			float d;
		};
		float q[4];
	};
	Quaternion()
	{
		a = 0;
		b = 0;
		c = 0;
		d = 0;
	}
	Quaternion(float _x, float _y, float _z, float _w)
	{
		a = _x;
		b = _y;
		c = _z;
		d = _w;
	}
	float norm()
	{
		return sqrtf(SQ(a) + SQ(b) + SQ(c) + SQ(d));
	}
	void Normalize()
	{
		float n = 1.0f / sqrtf(SQ(a) + SQ(b) + SQ(c) + SQ(d));
		a *= n;
		b *= n;
		c *= n;
		d *= n;
	}
	void Conjugate()
	{
		a = -a;
		b = -b;
		c = -c;
	}
	void invert()
	{
		Conjugate();
		Normalize();
	}
	Quaternion operator*(Quaternion q1)
	{
		Quaternion v;
		v.a = ((d * q1.a) + (a * q1.d) + (b * q1.c) - (c * q1.b));
		v.b = ((d * q1.b) - (a * q1.c) + (b * q1.d) + (c * q1.a));
		v.c = ((d * q1.c) + (a * q1.b) - (b * q1.a) + (c * q1.d));
		v.d = ((d * q1.d) - (a * q1.a) - (b * q1.b) - (c * q1.c));
		return v;
	}
	Matrix toMatrix()
	{
		float aa = a * a;
		float ab = a * b;
		float ac = a * c;
		float bb = b * b;
		float cc = c * c;
		float bc = b * c;
		float da = d * a;
		float db = d * b;
		float dc = d * c;
		Matrix matrix;
		matrix[0] = 1.0f - 2.0f * (bb + cc);
		matrix[1] = 2.0f * (ab - dc);
		matrix[2] = 2.0f * (ac + db);
		matrix[3] = 0.0;
		matrix[4] = 2.0f * (ab + dc);
		matrix[5] = 1.0f - 2.0f * (aa + cc);
		matrix[6] = 2.0f * (bc - da);
		matrix[7] = 0.0;
		matrix[8] = 2.0f * (ac - db);
		matrix[9] = 2.0f * (bc + da);
		matrix[10] = 1.0f - 2.0f * (aa + bb);
		matrix[11] = 0.0;
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		matrix[15] = 1;
		return matrix;
	}
	void rotateAboutAxis(Vec3 pt, float angle, Vec3 axis)
	{
		Quaternion q1, p, qinv;
		q1.a = sinf(0.5f * angle) * axis.x;
		q1.b = sinf(0.5f * angle) * axis.y;
		q1.c = sinf(0.5f * angle) * axis.z;
		q1.d = cosf(0.5f * angle);
		p.a = pt.x;
		p.b = pt.y;
		p.c = pt.z;
		p.d = 0;
		qinv = q1;
		qinv.invert();
		q1 = q1 * p;
		q1 = q1 * qinv;
		a = q1.a;
		b = q1.b;
		c = q1.c;
		d = q1.d;
	}
	static Quaternion slerp(Quaternion q1, Quaternion q2, float t)
	{
		Quaternion qr;
		float dp = q1.a * q2.a + q1.b * q2.b + q1.c * q2.c + q1.d * q2.d;
		Quaternion q11 = dp < 0 ? -q1 : q1;
		dp = dp > 0 ? dp : -dp;
		float theta = acosf(clamp(dp, -1.0f, 1.0f));
		if (theta == 0)
		{
			return q1;
		}
		float d = sinf(theta);
		float a = sinf((1 - t) * theta);
		float b = sinf(t * theta);
		float coeff1 = a / d;
		float coeff2 = b / d;
		qr.a = coeff1 * q11.a + coeff2 * q2.a;
		qr.b = coeff1 * q11.b + coeff2 * q2.b;
		qr.c = coeff1 * q11.c + coeff2 * q2.c;
		qr.d = coeff1 * q11.d + coeff2 * q2.d;
		qr.Normalize();
		return qr;
	}
	Quaternion operator-()
	{
		return Quaternion(-a, -b, -c, -d);
	}
};

struct PRIM_VERTEX
{
	Vec3 position;
	Colour colour;
};


struct STATIC_VERTEX
{
	Vec3 pos;
	Vec3 normal;
	Vec3 tangent;
	float tu;
	float tv;
};

struct ANIMATED_VERTEX
{
	Vec3 pos;
	Vec3 normal;
	Vec3 tangent;
	float tu;
	float tv;
	unsigned int bonesIDs[4];
	float boneWeights[4];
};

