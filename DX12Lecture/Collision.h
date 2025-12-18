#pragma once
#include "Vec3.h"

#include <vector>
#include <cfloat>

// 碰撞体类型枚举
enum class CollisionShapeType
{
	None,       // 无碰撞
	AABB,       // 轴对齐包围盒
	Sphere      // 球体包围盒
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