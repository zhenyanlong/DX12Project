#include "Collision.h"
#include "Actor.h"
Vec3 CollisionResolver::resolveSlidingCollision(Actor* const controlledActor, const Vec3& desiredMove, const std::vector<Actor*>& collidableActors, float epsilon)
{
	//if (desiredMove.lengthSq() < epsilon * epsilon)
	//	return Vec3(0, 0, 0);

	//Vec3 remainingMove = desiredMove;
	//const int maxIterations = 5; // 最大迭代次数（避免无限循环）

	//// 记录原始位置
	//Vec3 originalPos = controlledActor->getWorldPos();

	//for (int i = 0; i < maxIterations; i++)
	//{
	//	if (remainingMove.lengthSq() < epsilon * epsilon)
	//		break;

	//	// 测试移动后的位置
	//	Vec3 testPos = controlledActor->getWorldPos() + remainingMove;

	//	// 临时设置位置进行碰撞检测
	//	Vec3 currentPos = controlledActor->getWorldPos();
	//	controlledActor->setWorldPos(testPos);

	//	// 1. 找到最近的碰撞
	//	CollisionResult closestCollision;
	//	class Actor* closestActor = nullptr;
	//	float minDist = FLT_MAX;

	//	// 获取被控制Actor的世界空间碰撞体
	//	auto controlledWorldAABB = controlledActor->getWorldAABB();
	//	auto controlledWorldSphere = controlledActor->getWorldSphere();
	//	CollisionShapeType controlledShape = controlledActor->getCollisionShapeType();

	//	// 遍历所有可碰撞Actor，检测碰撞
	//	for (auto* actor : collidableActors)
	//	{
	//		if (!actor->isCollidable() || actor == dynamic_cast<Actor*>(controlledActor))
	//			continue; // 跳过自身和不可碰撞Actor

	//		CollisionResult collision;
	//		switch (actor->getCollisionShapeType())
	//		{
	//		case CollisionShapeType::AABB:
	//			if (controlledShape == CollisionShapeType::AABB)
	//				collision = CollisionDetector::checkAABBAABB(controlledWorldAABB, actor->getWorldAABB());
	//			else if (controlledShape == CollisionShapeType::Sphere)
	//				collision = CollisionDetector::checkSphereAABB(controlledWorldSphere, actor->getWorldAABB());
	//			break;
	//		case CollisionShapeType::Sphere:
	//			if (controlledShape == CollisionShapeType::AABB)
	//				collision = CollisionDetector::checkSphereAABB(actor->getWorldSphere(), controlledWorldAABB);
	//			else if (controlledShape == CollisionShapeType::Sphere)
	//				collision = CollisionDetector::checkSphereSphere(controlledWorldSphere, actor->getWorldSphere());
	//			break;
	//		default:
	//			continue;
	//		}

	//		// 记录最近的碰撞
	//		if (collision.isColliding)
	//		{
	//			// 计算碰撞点到被控制Actor的距离（简化为穿透深度的倒数）
	//			float dist = 1.0f / (collision.penetration + epsilon);
	//			if (dist < minDist)
	//			{
	//				minDist = dist;
	//				closestCollision = collision;
	//				closestActor = actor;
	//			}
	//		}
	//	}

	//	// 2. 无碰撞：直接返回剩余移动
	//	if (!closestCollision.isColliding)
	//		break;

	//	// 恢复原始位置
	//	controlledActor->setWorldPos(currentPos);

	//	// 无碰撞：直接应用移动
	//	if (!closestCollision.isColliding)
	//	{
	//		//Vec3 newPos = controlledActor->getWorldPos() + remainingMove;
	//		//controlledActor->setWorldPos(newPos);
	//		break;
	//	}

	//	// 有碰撞：分解移动向量，但允许小部分法向量方向的移动
	//	float dotProduct = Dot(remainingMove, closestCollision.normal);

	//	if (dotProduct < 0)
	//	{
	//		// 如果移动方向与法向量方向相同（尝试离开物体），允许部分移动
	//		Vec3 normalComponent = closestCollision.normal * (dotProduct * 1.f); // 允许30%
	//		Vec3 tangentComponent = remainingMove - closestCollision.normal * dotProduct;
	//		remainingMove = tangentComponent + normalComponent;
	//	}
	//	else
	//	{
	//		// 如果移动方向与法向量相反（尝试进入物体），完全移除该分量
	//		remainingMove = remainingMove - closestCollision.normal * dotProduct;
	//	}
	//	//// 3. 有碰撞：分离穿透（先消除重叠）
	//	////controlledActor->GetPosition() += closestCollision.normal * closestCollision.penetration;
	//	//Vec3 newPos = controlledActor->getWorldPos() + closestCollision.normal * closestCollision.penetration;
	//	//controlledActor->setWorldPos(newPos);
	//	//// 4. 分解移动向量：保留垂直于法向量的分量（滑动方向）
	//	//float dotProduct = remainingMove.dot(closestCollision.normal);
	//	/*float dotProduct = Dot(remainingMove, closestCollision.normal);
	//	remainingMove = remainingMove - closestCollision.normal * dotProduct;*/
	//	// 应用部分移动（防止卡住）
	//	float moveRatio = 0.0f; // 每次迭代移动剩余向量的70%
	//	Vec3 partialMove = remainingMove * moveRatio;
	//	Vec3 newPos = controlledActor->getWorldPos() + partialMove;
	//	controlledActor->setWorldPos(newPos);

	//	// 更新剩余移动
	//	remainingMove = remainingMove * (1.0f - moveRatio);
	//}
	////controlledActor->setWorldPos(originalPos);
	//return remainingMove;
	if (desiredMove.lengthSq() < epsilon * epsilon)
        return Vec3(0, 0, 0);

    Vec3 remainingMove = desiredMove;
    const int maxIterations = 8; // 增加迭代次数，处理多碰撞
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
            if (!actor->isCollidable() || actor == controlledActor)
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
