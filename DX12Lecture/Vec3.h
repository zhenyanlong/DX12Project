#pragma once
#include <cmath>  // 包含sqrtf、max等数学函数
#include <iostream>
//#define NOMINMAX
#define SQ(x) ((x) * (x))
#define PI       3.14159265358979323846
#define WIDTH 1280
#define HEIGHT 720
class Vec3
{
public:
	
		         
		struct { float x, y, z; };  // 仅保留x/y/z分量，移除w
	

	// 构造函数：适配三维分量
		Vec3() : x(0.f), y(0.f), z(0.f) {}
		Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	// 加法运算：仅处理x/y/z
	Vec3 operator+(const Vec3& pVec) const
	{
		return Vec3(x + pVec.x, y + pVec.y, z + pVec.z);
	}

	Vec3& operator+=(const Vec3& pVec)
	{
		x += pVec.x;
		y += pVec.y;
		z += pVec.z;
		return *this;
	}

	// 乘法运算（分量乘、标量乘）
	Vec3 operator*(const Vec3& pVec) const
	{
		return Vec3(x * pVec.x, y * pVec.y, z * pVec.z);
	}

	Vec3 operator*(const float val) const
	{
		return Vec3(x * val, y * val, z * val);
	}

	// 除法运算（分量除、标量除）
	Vec3 operator/(const Vec3& pVec) const
	{
		return Vec3(x / pVec.x, y / pVec.y, z / pVec.z);
	}

	Vec3 operator/(const float val) const
	{
		return Vec3(x / val, y / val, z / val);
	}

	Vec3& operator/=(const Vec3& pVec)
	{
		x /= pVec.x;
		y /= pVec.y;
		z /= pVec.z;
		return *this;
	}

	Vec3& operator/=(const float val)
	{
		x /= val;
		y /= val;
		z /= val;
		return *this;
	}

	// 负号运算（分量取反）
	Vec3 operator-() const
	{
		return Vec3(-x, -y, -z);
	}

	// 减法运算：补充const修饰，仅处理x/y/z
	Vec3 operator-(const Vec3& v) const
	{
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	Vec3& operator-=(const Vec3& pVec)
	{
		x -= pVec.x;
		y -= pVec.y;
		z -= pVec.z;
		return *this;
	}

	// 长度平方：仅计算x?+y?+z?（移除w?）
	float LengthSQ() const  // 补充const修饰，保证常对象可调用
	{
		return SQ(x) + SQ(y) + SQ(z);
	}

	// 长度（模长）
	float Length() const  // 补充const修饰
	{
		return sqrtf(LengthSQ());
	}

	// 归一化（返回新向量，不修改原向量）
	Vec3 normalize() const
	{
		float len = 1.0f / Length();  // 复用Length()，避免重复计算
		return Vec3(x * len, y * len, z * len);
	}

	// 归一化（修改原向量，返回原长度）
	float normalize_GetLength()
	{
		float length = Length();
		if (length == 0.f) return 0.f;  // 避免除零
		float inv_len = 1.0f / length;
		x *= inv_len;
		y *= inv_len;
		z *= inv_len;
		return length;
	}

	// 返回三个分量中的最大值
	//float Max() const
	//{
	//	return std::max(x, std::max(y, z));  // 显式调用std::max，避免歧义
	//}

	// 点积（静态方法）：移除divW，仅计算x1x2+y1y2+z1z2
	static float Dot(const Vec3& v1, const Vec3& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	// 叉积（三维标准叉积）：返回Vec3，移除w分量
	static Vec3 Cross(const Vec3& v1, const Vec3& v2)
	{
		return Vec3(
			v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x
		);
	}

	// 逐分量取最大值（静态方法）：仅处理x/y/z
	/*static Vec3 Max(const Vec3& v1, const Vec3& v2)
	{
		return Vec3(
			std::max(v1.x, v2.x),
			std::max(v1.y, v2.y),
			std::max(v1.z, v2.z)
		);
	}*/

	// 移除divW()：Vec3无w分量，该方法无意义
};

// 标量左乘（如 5 * Vec3(1,2,3)）：补充全局运算符
inline Vec3 operator*(float val, const Vec3& vec)
{
	return vec * val;
}

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

class Matrix
{
public:
	union
	{
		float a[4][4];	// a[列][行]（列优先存储，匹配HLSL）
		float m[16];
	};
	Matrix()
	{
		m[0] = 0; m[1] = 0; m[2] = 0; m[3] = 0;
		m[4] = 0; m[5] = 0; m[6] = 0; m[7] = 0;
		m[8] = 0; m[9] = 0; m[10] = 0; m[11] = 0;
		m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 0;
	}
	// 单位矩阵
	static Matrix Identity()
	{
		Matrix mat;
		mat.m[0] = 1.0f;  // 第0列第0行
		mat.m[5] = 1.0f;  // 第1列第1行
		mat.m[10] = 1.0f; // 第2列第2行
		mat.m[15] = 1.0f; // 第3列第3行
		return mat;
	}

	Vec4 mul(const Vec4& v) const
	{
		return Vec4(
			(v.x * m[0] + v.y * m[4] + v.z * m[8] + v.w * m[12]),
			(v.x * m[1] + v.y * m[5] + v.z * m[9] + v.w * m[13]),
			(v.x * m[2] + v.y * m[6] + v.z * m[10] + v.w * m[14]),
			(v.x * m[3] + v.y * m[7] + v.z * m[11] + v.w * m[15]));
	}

	// 矩阵乘法（列优先：this * other，结果仍为列优先）
	Matrix mul(const Matrix& other) const
	{
		Matrix result;
		for (int col = 0; col < 4; col++) 
		{
			for (int row = 0; row < 4; row++)
			{
				float sum = 0.0f;
				for (int k = 0; k < 4; k++) 
				{
					sum += this->a[k][row] * other.a[col][k];
				}
				result.a[col][row] = sum;
			}
		}
		return result;
	}

	// 世界矩阵（平移矩阵，列优先）
	static Matrix SetPositionMatrix(const Vec4& InPoint)
	{
		Matrix mat = Identity();
		mat.m[3] = InPoint.x; // 第3列第0行（x平移）
		mat.m[7] = InPoint.y; // 第3列第1行（y平移）
		mat.m[11] = InPoint.z; // 第3列第2行（z平移）
		return mat;
	}

	// 透视投影矩阵（DirectX左手坐标系，z范围[0,1]，匹配HLSL）
	static Matrix GetProjectionMatrix(float fovDegree, float InNear, float InFar)
	{
		Matrix mat;
		float fovRad = fovDegree * PI / 180.0f;
		float aspect = WIDTH / (float)HEIGHT; // 强制float除法
		float tanHalfFov = std::tan(fovRad / 2.0f);

		mat.m[0] = 1.0f / (aspect * tanHalfFov); // 第0列第0行
		mat.m[5] = 1.0f / tanHalfFov;            // 第1列第1行
		mat.m[10] = InFar / (InFar - InNear);    // 第2列第2行（左手坐标系修正）
		mat.m[11] = -(InFar * InNear) / (InFar - InNear); // 第2列第3行
		mat.m[14] = 1.0f;                        // 第3列第2行
		mat.m[15] = 0.0f;                        // 第3列第3行
		return mat;
	}

	// 观察矩阵（LookAt，列优先，匹配HLSL）
	static Matrix GetLookAtMatrix(const Vec4& InPos, const Vec4& InTo, const Vec4& InUp)
	{
		Vec4 pos = InPos; pos.divW();
		Vec4 to = InTo; to.divW();
		Vec4 up = InUp; up.divW();

		// 相机朝向：向前向量（指向目标，取反后为观察方向）
		Vec4 forward = (to - pos).Normalize();
		Vec4 right = Vec4::Cross(forward, up).Normalize(); // 右向量（修正叉乘顺序）
		up = Vec4::Cross(right, forward).Normalize();      // 修正上向量（正交化）

		// 观察矩阵 = 旋转矩阵（right/up/-forward） + 平移矩阵（-pos在相机空间的投影）
		Matrix mat;
		// 第0列（right向量）
		mat.m[0] = right.x;
		mat.m[1] = right.y;
		mat.m[2] = right.z;
		mat.m[3] = -Vec4::Dot(right, pos);
		// 第1列（up向量）
		mat.m[4] = up.x;
		mat.m[5] = up.y;
		mat.m[6] = up.z;
		mat.m[7] = -Vec4::Dot(up, pos);
		// 第2列（-forward向量，相机看向-z方向）
		mat.m[8] = forward.x;
		mat.m[9] = forward.y;
		mat.m[10] = forward.z;
		mat.m[11] = -Vec4::Dot(forward, pos);
		// 第3列（齐次项）
		mat.m[12] = 0.0f;
		mat.m[13] = 0.0f;
		mat.m[14] = 0.0f;
		mat.m[15] = 1.0f;
		return mat;
	}

	// 矩阵求逆（简化版，仅支持 affine 矩阵，用于法线矩阵计算）
	Matrix Inverse() const
	{
		Matrix inv = Identity();
		// 1. 逆旋转矩阵 = 旋转矩阵的转置（正交矩阵性质）
		inv.m[0] = m[0]; inv.m[4] = m[1]; inv.m[8] = m[2];
		inv.m[1] = m[4]; inv.m[5] = m[5]; inv.m[9] = m[6];
		inv.m[2] = m[8]; inv.m[6] = m[9]; inv.m[10] = m[10];
		// 2. 逆平移 = -旋转矩阵转置 * 原平移
		Vec4 trans(m[12], m[13], m[14], 0);
		Vec4 invTrans = inv.mul(trans) * -1.0f;
		inv.m[12] = invTrans.x;
		inv.m[13] = invTrans.y;
		inv.m[14] = invTrans.z;
		return inv;
	}

	// 矩阵转置
	Matrix Transpose() const
	{
		Matrix trans;
		for (int col = 0; col < 4; col++)
		{
			for (int row = 0; row < 4; row++)
			{
				trans.a[row][col] = a[col][row];
			}
		}
		return trans;
	}
	void Transform(Vec4& InPoint)
	{
		InPoint = mul(InPoint);
		/*InPoint.x *= m[0];
		InPoint.y *= m[5];
		InPoint.w = InPoint.z;
		InPoint.z = m[10] * InPoint.z + m[11];*/

		//InPoint.w = 1 / InPoint.w;
		//InPoint.divW();



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
