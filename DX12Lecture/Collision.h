#pragma once
#include "Vec3.h"

#include <vector>
#include <cfloat>


enum class CollisionShapeType
{
	None,       
	AABB,       
	Sphere,      
	OBB         // Directional bounding box (suitable for rotating objects)
};

// Collision detection result
struct CollisionResult
{
	bool isColliding = false;   // if collide
	Vec3 normal = Vec3(0, 1, 0); // collision normal
	float penetration = 0.0f;   // Penetration depth
};


class AABB
{
public:
	Vec3 min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vec3 max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	
	void reset()
	{
		min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	}

	// extend AABB with point
	void extend(const Vec3& point)
	{
		min.x = std::min(min.x, point.x);
		min.y = std::min(min.y, point.y);
		min.z = std::min(min.z, point.z);
		max.x = std::max(max.x, point.x);
		max.y = std::max(max.y, point.y);
		max.z = std::max(max.z, point.z);
	}

	
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

	
	Vec3 getCenter() const { return (min + max) * 0.5f; }

	
	Vec3 getSize() const { return max - min; }

	
	Vec3 getHalfExtents() const { return (max - min) * 0.5f; }
};

class OBB
{
public:
	Vec3 center;         
	Vec3 xAxis;           
	Vec3 yAxis;           
	Vec3 zAxis;           
	Vec3 halfExtents;     

	OBB() = default;

	// Convert local AABB to a world OBB with rotation.
	static OBB fromAABB(const AABB& localAABB, Matrix worldMat)
	{
		OBB obb;
		
		Vec3 localCenter = localAABB.getCenter();
		Vec3 localHalfExtents = localAABB.getHalfExtents();

		
		obb.center = worldMat.mulPoint(localCenter);

		
		Vec3 axisX(worldMat.m[0], worldMat.m[4], worldMat.m[8]);
		Vec3 axisY(worldMat.m[1], worldMat.m[5], worldMat.m[9]);
		Vec3 axisZ(worldMat.m[2], worldMat.m[6], worldMat.m[10]);

		
		float scaleX = axisX.length();
		float scaleY = axisY.length();
		float scaleZ = axisZ.length();

		
		obb.xAxis = scaleX > 1e-6f ? axisX / scaleX : Vec3(1, 0, 0);
		obb.yAxis = scaleY > 1e-6f ? axisY / scaleY : Vec3(0, 1, 0);
		obb.zAxis = scaleZ > 1e-6f ? axisZ / scaleZ : Vec3(0, 0, 1);

		
		obb.halfExtents.x = localHalfExtents.x * scaleX;
		obb.halfExtents.y = localHalfExtents.y * scaleY;
		obb.halfExtents.z = localHalfExtents.z * scaleZ;

		return obb;
	}

	
	std::vector<Vec3> getVertices() const
	{
		std::vector<Vec3> vertices;
		vertices.reserve(8);
		
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

// Projection
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

// overlap
static float getOverlap(float minA, float maxA, float minB, float maxB)
{
	if (maxA < minB || maxB < minA)
		return -1.0f; 
	return std::min(maxA, maxB) - std::max(minA, minB);
}

class Sphere
{
public:
	Vec3 centre = Vec3(0, 0, 0); 
	float radius = 0.0f;         

	Sphere() = default;
	Sphere(Vec3 centerpoint, float rad)
	{
		centre = centerpoint;
		radius = rad;
	}

	
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


class Ray
{
public:
	Vec3 o;        
	Vec3 dir;       
	Vec3 invdir;    

	Ray() = default;
	Ray(const Vec3& origin, const Vec3& direction)
	{
		init(origin, direction);
	}

	// init ray
	void init(const Vec3& origin, const Vec3& direction)
	{
		o = origin;
		dir = direction.normalize();
		invdir = Vec3(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z);
	}

	// Obtain the point at a distance t from the ray
	Vec3 at(float t) const { return o + dir * t; }
};

// Collision Detection Toolkit
class CollisionDetector
{
public:
	// AABB-AABB
	static CollisionResult checkAABBAABB(const AABB& aabbA, const AABB& aabbB)
	{
		CollisionResult result;

		
		float penetrationX = std::min(aabbA.max.x - aabbB.min.x, aabbB.max.x - aabbA.min.x);
		float penetrationY = std::min(aabbA.max.y - aabbB.min.y, aabbB.max.y - aabbA.min.y);
		float penetrationZ = std::min(aabbA.max.z - aabbB.min.z, aabbB.max.z - aabbA.min.z);

		
		if (penetrationX > 0 && penetrationY > 0 && penetrationZ > 0)
		{
			result.isColliding = true;
			
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

	// Sphere-Sphere
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
			result.normal = dir / dist; 
			result.penetration = radiusSum - dist;
		}

		return result;
	}

	// Sphere-AABB
	static CollisionResult checkSphereAABB(const Sphere& sphere, const AABB& aabb)
	{
		CollisionResult result;

		
		Vec3 closestPoint = sphere.centre;
		closestPoint.x = std::clamp(closestPoint.x, aabb.min.x, aabb.max.x);
		closestPoint.y = std::clamp(closestPoint.y, aabb.min.y, aabb.max.y);
		closestPoint.z = std::clamp(closestPoint.z, aabb.min.z, aabb.max.z);

		
		Vec3 dir = closestPoint - sphere.centre;
		float distSq = dir.lengthSq();

		if (distSq < sphere.radius * sphere.radius)
		{
			result.isColliding = true;
			float dist = sqrt(distSq);
			result.normal = dist > 0 ? dir / dist : Vec3(0, 1, 0); 
			result.penetration = sphere.radius - dist;
		}

		return result;
	}

	// Ray-AABB
	static CollisionResult checkRayAABB(const Ray& ray, const AABB& aabb, float& t)
	{
		CollisionResult result;

		Vec3 s = (aabb.min - ray.o) * ray.invdir;
		Vec3 l = (aabb.max - ray.o) * ray.invdir;

		
		Vec3 tMin = Vec3(std::min(s.x, l.x), std::min(s.y, l.y), std::min(s.z, l.z));
		Vec3 tMax = Vec3(std::max(s.x, l.x), std::max(s.y, l.y), std::max(s.z, l.z));

		
		float tNear = std::max({ tMin.x, tMin.y, tMin.z });
		float tFar = std::min({ tMax.x, tMax.y, tMax.z });

		if (tNear < tFar && tFar > 0)
		{
			result.isColliding = true;
			t = tNear;
			
			if (tNear == tMin.x)
				result.normal = ray.dir.x > 0 ? Vec3(-1, 0, 0) : Vec3(1, 0, 0);
			else if (tNear == tMin.y)
				result.normal = ray.dir.y > 0 ? Vec3(0, -1, 0) : Vec3(0, 1, 0);
			else
				result.normal = ray.dir.z > 0 ? Vec3(0, 0, -1) : Vec3(0, 0, 1);
		}

		return result;
	}

	// Ray-Sphere
	static CollisionResult checkRaySphere(const Ray& ray, const Sphere& sphere, float& t)
	{
		CollisionResult result;

		Vec3 oc = ray.o - sphere.centre;
		float a = ray.dir.lengthSq();
		
		float b = 2.0f * Dot(oc, ray.dir);
		float c = oc.lengthSq() - sphere.radius * sphere.radius;
		float discriminant = b * b - 4 * a * c;

		if (discriminant > 0)
		{
			float sqrtDisc = sqrt(discriminant);
			float t1 = (-b - sqrtDisc) / (2 * a);
			float t2 = (-b + sqrtDisc) / (2 * a);

			
			if (t1 > 0)
				t = t1;
			else if (t2 > 0)
				t = t2;
			else
				return result; 

			result.isColliding = true;
			Vec3 hitPoint = ray.at(t);
			result.normal = (hitPoint - sphere.centre).normalize();
		}

		return result;
	}

	// OBB-OBB
	static CollisionResult checkOBBOBB(const OBB& obbA, const OBB& obbB)
	{
		CollisionResult result;
		float minPenetration = FLT_MAX;
		Vec3 bestAxis; 

		// collect all axis
		std::vector<Vec3> axes;
		// 
		axes.push_back(obbA.xAxis);
		axes.push_back(obbA.yAxis);
		axes.push_back(obbA.zAxis);
		// 
		axes.push_back(obbB.xAxis);
		axes.push_back(obbB.yAxis);
		axes.push_back(obbB.zAxis);
		// 
		axes.push_back(Cross(obbA.xAxis, obbB.xAxis));
		axes.push_back(Cross(obbA.xAxis, obbB.yAxis));
		axes.push_back(Cross(obbA.xAxis, obbB.zAxis));
		axes.push_back(Cross(obbA.yAxis, obbB.xAxis));
		axes.push_back(Cross(obbA.yAxis, obbB.yAxis));
		axes.push_back(Cross(obbA.yAxis, obbB.zAxis));
		axes.push_back(Cross(obbA.zAxis, obbB.xAxis));
		axes.push_back(Cross(obbA.zAxis, obbB.yAxis));
		axes.push_back(Cross(obbA.zAxis, obbB.zAxis));

		// Traverse all axes
		for (auto& axis : axes)
		{
			
			if (axis.lengthSq() < 1e-6f)
				continue;
			axis = axis.normalize(); 

			
			float minA, maxA, minB, maxB;
			projectPoints(obbA.getVertices(), axis, minA, maxA);
			projectPoints(obbB.getVertices(), axis, minB, maxB);

			
			float overlap = getOverlap(minA, maxA, minB, maxB);
			if (overlap < 0)
				return result; 

			
			if (overlap < minPenetration)
			{
				minPenetration = overlap;
				bestAxis = axis;
			}
		}

		
		result.isColliding = true;
		result.penetration = minPenetration;

		
		Vec3 dir = obbB.center - obbA.center;
		if (Dot(bestAxis, dir) < 0)
			bestAxis = -bestAxis;
		result.normal = bestAxis;

		return result;
	}
	// OBB-Sphere
	static CollisionResult checkOBBSphere(const OBB& obb, const Sphere& sphere)
	{
		CollisionResult result;

		// Find the point on the OBB that is closest to the center of sphere
		Vec3 dir = sphere.centre - obb.center;
		Vec3 closestPoint = obb.center;

		float proj = Dot(dir, obb.xAxis);
		proj = std::clamp(proj, -obb.halfExtents.x, obb.halfExtents.x);
		closestPoint += obb.xAxis * proj;

		proj = Dot(dir, obb.yAxis);
		proj = std::clamp(proj, -obb.halfExtents.y, obb.halfExtents.y);
		closestPoint += obb.yAxis * proj;

		proj = Dot(dir, obb.zAxis);
		proj = std::clamp(proj, -obb.halfExtents.z, obb.halfExtents.z);
		closestPoint += obb.zAxis * proj;

		// Calculate the distance from the center of the sphere to the nearest point
		Vec3 diff = closestPoint - sphere.centre;
		float distSq = diff.lengthSq();

		if (distSq < sphere.radius * sphere.radius)
		{
			
			result.isColliding = true;
			float dist = sqrt(distSq);
			result.penetration = sphere.radius - dist;
			result.normal = dist > 1e-6f ? diff / dist : Vec3(0, 1, 0); 

			
			if (Dot(result.normal, sphere.centre - obb.center) < 0)
				result.normal = -result.normal;
		}

		return result;
	}
};

class Actor;
// Collision Response Toolkit
class CollisionResolver
{
public:
	// Sliding collision response
	static Vec3 resolveSlidingCollision(
		Actor* const controlledActor,
		const Vec3& desiredMove,
		const std::vector<Actor*>& collidableActors,
		float epsilon = 0.001f
	);
	// check collision, return all actors that triggers the collision 
	static std::vector<Actor*> CheckCollision(Actor* const controlledActor, const std::vector<Actor*>& collidableActors);
};