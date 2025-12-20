#pragma once
#include "Vec3.h"

#include <vector>
#include <cfloat>

// 碰撞体类型枚举
enum class CollisionShapeType
{
	None,       // 无碰撞
	AABB,       // 轴对齐包围盒
	Sphere,      // 球体包围盒
	OBB         // 定向包围盒（适合旋转的动态物体）
};

// 碰撞检测结果（包含碰撞状态、法向量、穿透深度）
struct CollisionResult
{
	bool isColliding = false;   // 是否碰撞
	Vec3 normal = Vec3(0, 1, 0); // 碰撞法向量（指向被碰撞体外部）
	float penetration = 0.0f;   // 穿透深度（需分离的距离）
};

// 轴对齐包围盒（AABB，局部空间）
class AABB
{
public:
	Vec3 min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vec3 max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// 重置AABB（初始为无效状态）
	void reset()
	{
		min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	}

	// 用点扩展AABB（计算模型顶点的包围盒）
	void extend(const Vec3& point)
	{
		min.x = std::min(min.x, point.x);
		min.y = std::min(min.y, point.y);
		min.z = std::min(min.z, point.z);
		max.x = std::max(max.x, point.x);
		max.y = std::max(max.y, point.y);
		max.z = std::max(max.z, point.z);
	}

	// 获取AABB的8个顶点（用于世界空间转换）
	std::vector<Vec3> getVertices() const
	{
		return {
			Vec3(min.x, min.y, min.z),
			Vec3(min.x, min.y, max.z),
			Vec3(min.x, max.y, min.z),
			Vec3(min.x, max.y, max.z),
			Vec3(max.x, min.y, min.z),
			Vec3(max.x, min.y, max.z),
			Vec3(max.x, max.y, min.z),
			Vec3(max.x, max.y, max.z)
		};
	}

	// 获取AABB中心
	Vec3 getCenter() const { return (min + max) * 0.5f; }

	// 获取AABB大小
	Vec3 getSize() const { return max - min; }

	// 获取AABB半长
	Vec3 getHalfExtents() const { return (max - min) * 0.5f; }
};

class OBB
{
public:
	Vec3 center;          // 世界空间中心
	Vec3 xAxis;           // 局部X轴（单位向量，世界空间）
	Vec3 yAxis;           // 局部Y轴（单位向量，世界空间）
	Vec3 zAxis;           // 局部Z轴（单位向量，世界空间）
	Vec3 halfExtents;     // 沿每个轴的半长度（世界空间）

	OBB() = default;

	// 从局部AABB和世界矩阵生成世界OBB（核心：将局部AABB转换为带旋转的世界OBB）
	static OBB fromAABB(const AABB& localAABB, Matrix worldMat)
	{
		OBB obb;
		// 1. 局部AABB的中心和半长
		Vec3 localCenter = localAABB.getCenter();
		Vec3 localHalfExtents = localAABB.getHalfExtents();

		// 2. 将局部中心转换到世界空间（包含平移、旋转、缩放）
		obb.center = worldMat.mulPoint(localCenter);

		// 3. 提取世界矩阵的旋转+缩放轴（前三列）
		Vec3 axisX(worldMat.m[0], worldMat.m[4], worldMat.m[8]);
		Vec3 axisY(worldMat.m[1], worldMat.m[5], worldMat.m[9]);
		Vec3 axisZ(worldMat.m[2], worldMat.m[6], worldMat.m[10]);

		// 4. 计算每个轴的缩放因子（轴的长度）
		float scaleX = axisX.length();
		float scaleY = axisY.length();
		float scaleZ = axisZ.length();

		// 5. 设置OBB的局部轴（单位向量，去除缩放影响）
		obb.xAxis = scaleX > 1e-6f ? axisX / scaleX : Vec3(1, 0, 0);
		obb.yAxis = scaleY > 1e-6f ? axisY / scaleY : Vec3(0, 1, 0);
		obb.zAxis = scaleZ > 1e-6f ? axisZ / scaleZ : Vec3(0, 0, 1);

		// 6. 设置OBB的半长（局部半长 × 缩放因子）
		obb.halfExtents.x = localHalfExtents.x * scaleX;
		obb.halfExtents.y = localHalfExtents.y * scaleY;
		obb.halfExtents.z = localHalfExtents.z * scaleZ;

		return obb;
	}

	// 获取OBB的8个顶点（用于碰撞检测的投影计算）
	std::vector<Vec3> getVertices() const
	{
		std::vector<Vec3> vertices;
		vertices.reserve(8);
		// 生成逻辑：center ± xAxis*halfExtents.x ± yAxis*halfExtents.y ± zAxis*halfExtents.z
		for (int i = 0; i < 2; i++)
		{
			float x = i ? halfExtents.x : -halfExtents.x;
			for (int j = 0; j < 2; j++)
			{
				float y = j ? halfExtents.y : -halfExtents.y;
				for (int k = 0; k < 2; k++)
				{
					float z = k ? halfExtents.z : -halfExtents.z;
					vertices.push_back(center + xAxis * x + yAxis * y + zAxis * z);
				}
			}
		}
		return vertices;
	}
};

// 辅助函数：将点集投影到轴上，返回最小/最大投影值
static void projectPoints(const std::vector<Vec3>& points, const Vec3& axis, float& minProj, float& maxProj)
{
	minProj = FLT_MAX;
	maxProj = -FLT_MAX;
	for (const auto& p : points)
	{
		float proj = Dot(p, axis);
		minProj = std::min(minProj, proj);
		maxProj = std::max(maxProj, proj);
	}
}

// 辅助函数：计算两个投影区间的重叠深度（负数表示无重叠）
static float getOverlap(float minA, float maxA, float minB, float maxB)
{
	if (maxA < minB || maxB < minA)
		return -1.0f; // 无重叠
	return std::min(maxA, maxB) - std::max(minA, minB); // 重叠深度
}
// 球体包围盒（局部空间）
class Sphere
{
public:
	Vec3 centre = Vec3(0, 0, 0); // 球心
	float radius = 0.0f;         // 半径

	Sphere() = default;
	Sphere(Vec3 centerpoint, float rad)
	{
		centre = centerpoint;
		radius = rad;
	}

	// 用点扩展球体（计算模型顶点的包围球）
	void extend(const Vec3& point)
	{
		Vec3 dir = point - centre;
		float dist = dir.length();
		if (dist > radius)
		{
			radius = dist;
		}
	}
};

// 射线（用于射线检测）
class Ray
{
public:
	Vec3 o;        // 射线原点
	Vec3 dir;       // 射线方向（单位向量）
	Vec3 invdir;    // 方向的倒数（优化射线检测）

	Ray() = default;
	Ray(const Vec3& origin, const Vec3& direction)
	{
		init(origin, direction);
	}

	// 初始化射线（自动归一化方向并计算invdir）
	void init(const Vec3& origin, const Vec3& direction)
	{
		o = origin;
		dir = direction.normalize();
		invdir = Vec3(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z);
	}

	// 获取射线t距离处的点
	Vec3 at(float t) const { return o + dir * t; }
};

// 碰撞检测工具类（静态方法，所有检测基于世界空间）
class CollisionDetector
{
public:
	// 1. AABB-AABB碰撞检测（PPT算法）
	static CollisionResult checkAABBAABB(const AABB& aabbA, const AABB& aabbB)
	{
		CollisionResult result;

		// 计算各轴的穿透深度
		float penetrationX = std::min(aabbA.max.x - aabbB.min.x, aabbB.max.x - aabbA.min.x);
		float penetrationY = std::min(aabbA.max.y - aabbB.min.y, aabbB.max.y - aabbA.min.y);
		float penetrationZ = std::min(aabbA.max.z - aabbB.min.z, aabbB.max.z - aabbA.min.z);

		// 所有轴穿透深度均为正，则碰撞
		if (penetrationX > 0 && penetrationY > 0 && penetrationZ > 0)
		{
			result.isColliding = true;
			// 取最小穿透深度对应的轴作为法向量
			float minPenetration = std::min({ penetrationX, penetrationY, penetrationZ });
			if (minPenetration == penetrationX)
				result.normal = (aabbA.getCenter().x < aabbB.getCenter().x) ? Vec3(-1, 0, 0) : Vec3(1, 0, 0);
			else if (minPenetration == penetrationY)
				result.normal = (aabbA.getCenter().y < aabbB.getCenter().y) ? Vec3(0, -1, 0) : Vec3(0, 1, 0);
			else
				result.normal = (aabbA.getCenter().z < aabbB.getCenter().z) ? Vec3(0, 0, -1) : Vec3(0, 0, 1);
			result.penetration = minPenetration;
		}

		return result;
	}

	// 2. Sphere-Sphere碰撞检测（PPT算法）
	static CollisionResult checkSphereSphere(const Sphere& sphereA, const Sphere& sphereB)
	{
		CollisionResult result;

		Vec3 dir = sphereB.centre - sphereA.centre;
		float distSq = dir.lengthSq();
		float radiusSum = sphereA.radius + sphereB.radius;

		if (distSq < radiusSum * radiusSum)
		{
			result.isColliding = true;
			float dist = sqrt(distSq);
			result.normal = dir / dist; // 法向量指向sphereB
			result.penetration = radiusSum - dist;
		}

		return result;
	}

	// 3. Sphere-AABB碰撞检测（PPT算法）
	static CollisionResult checkSphereAABB(const Sphere& sphere, const AABB& aabb)
	{
		CollisionResult result;

		// 找到AABB上离球心最近的点
		Vec3 closestPoint = sphere.centre;
		closestPoint.x = std::clamp(closestPoint.x, aabb.min.x, aabb.max.x);
		closestPoint.y = std::clamp(closestPoint.y, aabb.min.y, aabb.max.y);
		closestPoint.z = std::clamp(closestPoint.z, aabb.min.z, aabb.max.z);

		// 计算球心到最近点的距离
		Vec3 dir = closestPoint - sphere.centre;
		float distSq = dir.lengthSq();

		if (distSq < sphere.radius * sphere.radius)
		{
			result.isColliding = true;
			float dist = sqrt(distSq);
			result.normal = dist > 0 ? dir / dist : Vec3(0, 1, 0); // 避免除零
			result.penetration = sphere.radius - dist;
		}

		return result;
	}

	// 4. Ray-AABB碰撞检测（PPT Slab Test算法）
	static CollisionResult checkRayAABB(const Ray& ray, const AABB& aabb, float& t)
	{
		CollisionResult result;

		Vec3 s = (aabb.min - ray.o) * ray.invdir;
		Vec3 l = (aabb.max - ray.o) * ray.invdir;

		// 计算t的范围
		Vec3 tMin = Vec3(std::min(s.x, l.x), std::min(s.y, l.y), std::min(s.z, l.z));
		Vec3 tMax = Vec3(std::max(s.x, l.x), std::max(s.y, l.y), std::max(s.z, l.z));

		// 射线与AABB相交的条件：最大最小t重叠
		float tNear = std::max({ tMin.x, tMin.y, tMin.z });
		float tFar = std::min({ tMax.x, tMax.y, tMax.z });

		if (tNear < tFar && tFar > 0)
		{
			result.isColliding = true;
			t = tNear; // 取最近交点
			// 计算碰撞法向量（基于哪个轴先相交）
			if (tNear == tMin.x)
				result.normal = ray.dir.x > 0 ? Vec3(-1, 0, 0) : Vec3(1, 0, 0);
			else if (tNear == tMin.y)
				result.normal = ray.dir.y > 0 ? Vec3(0, -1, 0) : Vec3(0, 1, 0);
			else
				result.normal = ray.dir.z > 0 ? Vec3(0, 0, -1) : Vec3(0, 0, 1);
		}

		return result;
	}

	// 5. Ray-Sphere碰撞检测（PPT二次方程算法）
	static CollisionResult checkRaySphere(const Ray& ray, const Sphere& sphere, float& t)
	{
		CollisionResult result;

		Vec3 oc = ray.o - sphere.centre;
		float a = ray.dir.lengthSq();
		// float b = 2.0f * oc.dot(ray.dir);
		float b = 2.0f * Dot(oc, ray.dir);
		float c = oc.lengthSq() - sphere.radius * sphere.radius;
		float discriminant = b * b - 4 * a * c;

		if (discriminant > 0)
		{
			float sqrtDisc = sqrt(discriminant);
			float t1 = (-b - sqrtDisc) / (2 * a);
			float t2 = (-b + sqrtDisc) / (2 * a);

			// 取最近的有效交点（t>0）
			if (t1 > 0)
				t = t1;
			else if (t2 > 0)
				t = t2;
			else
				return result; // 交点在射线后方

			result.isColliding = true;
			Vec3 hitPoint = ray.at(t);
			result.normal = (hitPoint - sphere.centre).normalize();
		}

		return result;
	}

	// OBB-OBB碰撞检测（分离轴定理SAT）
	static CollisionResult checkOBBOBB(const OBB& obbA, const OBB& obbB)
	{
		CollisionResult result;
		float minPenetration = FLT_MAX;
		Vec3 bestAxis; // 最小重叠深度对应的轴（碰撞法向量）

		// 步骤1：收集所有潜在的分离轴（共15个：3+3+9）
		std::vector<Vec3> axes;
		// 1. OBB A的三个局部轴
		axes.push_back(obbA.xAxis);
		axes.push_back(obbA.yAxis);
		axes.push_back(obbA.zAxis);
		// 2. OBB B的三个局部轴
		axes.push_back(obbB.xAxis);
		axes.push_back(obbB.yAxis);
		axes.push_back(obbB.zAxis);
		// 3. A的轴与B的轴的叉积（9个，可能为零向量）
		axes.push_back(Cross(obbA.xAxis, obbB.xAxis));
		axes.push_back(Cross(obbA.xAxis, obbB.yAxis));
		axes.push_back(Cross(obbA.xAxis, obbB.zAxis));
		axes.push_back(Cross(obbA.yAxis, obbB.xAxis));
		axes.push_back(Cross(obbA.yAxis, obbB.yAxis));
		axes.push_back(Cross(obbA.yAxis, obbB.zAxis));
		axes.push_back(Cross(obbA.zAxis, obbB.xAxis));
		axes.push_back(Cross(obbA.zAxis, obbB.yAxis));
		axes.push_back(Cross(obbA.zAxis, obbB.zAxis));

		// 步骤2：遍历所有轴，检查是否存在分离轴
		for (auto& axis : axes)
		{
			// 忽略零向量（叉积可能为零）
			if (axis.lengthSq() < 1e-6f)
				continue;
			axis = axis.normalize(); // 单位化轴

			// 投影OBB A和B的顶点到当前轴
			float minA, maxA, minB, maxB;
			projectPoints(obbA.getVertices(), axis, minA, maxA);
			projectPoints(obbB.getVertices(), axis, minB, maxB);

			// 检查是否分离（无重叠）
			float overlap = getOverlap(minA, maxA, minB, maxB);
			if (overlap < 0)
				return result; // 存在分离轴，无碰撞

			// 记录最小重叠深度和对应轴
			if (overlap < minPenetration)
			{
				minPenetration = overlap;
				bestAxis = axis;
			}
		}

		// 所有轴都有重叠，发生碰撞
		result.isColliding = true;
		result.penetration = minPenetration;

		// 调整法向量方向（指向obbA的外部）
		Vec3 dir = obbB.center - obbA.center;
		if (Dot(bestAxis, dir) < 0)
			bestAxis = -bestAxis;
		result.normal = bestAxis;

		return result;
	}
	// OBB-Sphere碰撞检测
	static CollisionResult checkOBBSphere(const OBB& obb, const Sphere& sphere)
	{
		CollisionResult result;

		// 步骤1：找到OBB上离球心最近的点
		Vec3 dir = sphere.centre - obb.center;
		Vec3 closestPoint = obb.center;

		// 投影dir到OBB的每个轴，并限制在半长范围内
		float proj = Dot(dir, obb.xAxis);
		proj = std::clamp(proj, -obb.halfExtents.x, obb.halfExtents.x);
		closestPoint += obb.xAxis * proj;

		proj = Dot(dir, obb.yAxis);
		proj = std::clamp(proj, -obb.halfExtents.y, obb.halfExtents.y);
		closestPoint += obb.yAxis * proj;

		proj = Dot(dir, obb.zAxis);
		proj = std::clamp(proj, -obb.halfExtents.z, obb.halfExtents.z);
		closestPoint += obb.zAxis * proj;

		// 步骤2：计算球心到最近点的距离
		Vec3 diff = closestPoint - sphere.centre;
		float distSq = diff.lengthSq();

		if (distSq < sphere.radius * sphere.radius)
		{
			// 发生碰撞
			result.isColliding = true;
			float dist = sqrt(distSq);
			result.penetration = sphere.radius - dist;
			result.normal = dist > 1e-6f ? diff / dist : Vec3(0, 1, 0); // 避免除零

			// 调整法向量方向（指向球体外）
			if (Dot(result.normal, sphere.centre - obb.center) < 0)
				result.normal = -result.normal;
		}

		return result;
	}
};

class Actor;
// 碰撞响应工具类（实现滑动碰撞）
class CollisionResolver
{
public:
	// 滑动碰撞响应：调整期望移动向量，沿碰撞体边缘移动
	static Vec3 resolveSlidingCollision(
		Actor* const controlledActor,
		const Vec3& desiredMove,
		const std::vector<Actor*>& collidableActors,
		float epsilon = 0.001f
	);
	static std::vector<Actor*> CheckCollision(Actor* const controlledActor, const std::vector<Actor*>& collidableActors);
};