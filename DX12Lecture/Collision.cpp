#include "Collision.h"
#include "Actor.h"
Vec3 CollisionResolver::resolveSlidingCollision(Actor* const controlledActor, const Vec3& desiredMove, const std::vector<Actor*>& collidableActors, float epsilon)
{
	
	if (desiredMove.lengthSq() < epsilon * epsilon)
        return Vec3(0, 0, 0);

    Vec3 remainingMove = desiredMove;
    const int maxIterations = 3; // 增加迭代次数，处理多碰撞
    Vec3 currentPos = controlledActor->getWorldPos(); // 仅读取，不修改

    for (int i = 0; i < maxIterations; i++)
    {
        if (remainingMove.lengthSq() < epsilon * epsilon)
            break;

        // ===== 步骤1：收集所有碰撞的结果 =====
        std::vector<CollisionResult> allCollisions;
        CollisionShapeType controlledShape = controlledActor->getCollisionShapeType();

        // 临时计算测试位置（当前位置 + 剩余移动）
        Vec3 testPos = currentPos + remainingMove;
        controlledActor->setWorldPos(testPos); // 临时设置用于碰撞检测
        auto controlledWorldAABB = controlledActor->getWorldAABB();
        auto controlledWorldSphere = controlledActor->getWorldSphere();

        for (auto* actor : collidableActors)
        {
            if (!actor->isCollidable() || actor == controlledActor || actor->getActorType()!=ActorType::Static)
                continue;

            CollisionResult collision;
            switch (actor->getCollisionShapeType())
            {
            case CollisionShapeType::AABB:
                if (controlledShape == CollisionShapeType::AABB)
                    collision = CollisionDetector::checkAABBAABB(controlledWorldAABB, actor->getWorldAABB());
                else if (controlledShape == CollisionShapeType::Sphere)
                    collision = CollisionDetector::checkSphereAABB(controlledWorldSphere, actor->getWorldAABB());
                break;
            case CollisionShapeType::Sphere:
                if (controlledShape == CollisionShapeType::AABB)
                    collision = CollisionDetector::checkSphereAABB(actor->getWorldSphere(), controlledWorldAABB);
                else if (controlledShape == CollisionShapeType::Sphere)
                    collision = CollisionDetector::checkSphereSphere(controlledWorldSphere, actor->getWorldSphere());
                break;
            default:
                continue;
            }

            // 收集有效碰撞并修正法向量方向
            if (collision.isColliding)
            {
                Vec3 dirFromCollider = testPos - actor->getWorldPos();
                if (Dot(collision.normal, dirFromCollider) < 0)
                {
                    collision.normal = -collision.normal;
                }
                allCollisions.push_back(collision);
            }
        }

        // 恢复位置（仅临时用于检测，不修改实际位置）
        controlledActor->setWorldPos(currentPos);

        // ===== 步骤2：无碰撞时直接返回剩余移动 =====
        if (allCollisions.empty())
        {
            return remainingMove; // 关键：无碰撞时返回原始移动向量
        }

        // ===== 步骤3：合并所有碰撞的法向量 =====
        Vec3 combinedNormal = Vec3(0, 0, 0);
        for (const auto& col : allCollisions)
        {
            combinedNormal += col.normal * (col.penetration + epsilon);
        }
        if (combinedNormal.lengthSq() > epsilon * epsilon)
        {
            combinedNormal = combinedNormal.normalize();
        }
        else
        {
            combinedNormal = allCollisions[0].normal;
        }

        // ===== 步骤4：分解移动向量 =====
        float dotProduct = Dot(remainingMove, combinedNormal);
        if (dotProduct < 0)
        {
            remainingMove = remainingMove - combinedNormal * dotProduct;
        }

        // ===== 步骤5：限制移动长度 =====
        float maxMoveLength = desiredMove.length() * 1.5f;
		if (remainingMove.length() > maxMoveLength)
		{
			remainingMove = remainingMove.normalize() * maxMoveLength;
		}
    }

    return remainingMove;
}

std::vector<Actor*> CollisionResolver::CheckCollision(Actor* const controlledActor, const std::vector<Actor*>& collidableActors)
{
    
    std::vector<Actor*> collisions;

    for (auto* actor : collidableActors)
    {
        if (!actor->isCollidable() || actor == controlledActor|| (actor->getActorType()!=ActorType::Static&&actor->getActorType()!=ActorType::Enemy))
            continue;

        CollisionShapeType controlledShape = controlledActor->getCollisionShapeType();
		auto controlledWorldAABB = controlledActor->getWorldAABB();
		auto controlledWorldSphere = controlledActor->getWorldSphere();
        // check 
		CollisionResult collision;
		switch (actor->getCollisionShapeType())
		{
		case CollisionShapeType::AABB:
			if (controlledShape == CollisionShapeType::AABB)
				collision = CollisionDetector::checkAABBAABB(controlledWorldAABB, actor->getWorldAABB());
			else if (controlledShape == CollisionShapeType::Sphere)
				collision = CollisionDetector::checkSphereAABB(controlledWorldSphere, actor->getWorldAABB());
			break;
		case CollisionShapeType::Sphere:
			if (controlledShape == CollisionShapeType::AABB)
				collision = CollisionDetector::checkSphereAABB(actor->getWorldSphere(), controlledWorldAABB);
			else if (controlledShape == CollisionShapeType::Sphere)
				collision = CollisionDetector::checkSphereSphere(controlledWorldSphere, actor->getWorldSphere());
			break;
		default:
			continue;
		}

        if (collision.isColliding)
        {
            collisions.push_back(actor);
        }
    }
    return collisions;
}
