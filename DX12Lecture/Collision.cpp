#include "Collision.h"
#include "Actor.h"
Vec3 CollisionResolver::resolveSlidingCollision(Actor* const controlledActor, const Vec3& desiredMove, const std::vector<Actor*>& collidableActors, float epsilon)
{
	
	if (desiredMove.lengthSq() < epsilon * epsilon)
        return Vec3(0, 0, 0);

    Vec3 remainingMove = desiredMove;
    const int maxIterations = 3; // max Iteration count
    Vec3 currentPos = controlledActor->getWorldPos(); 

    for (int i = 0; i < maxIterations; i++)
    {
        if (remainingMove.lengthSq() < epsilon * epsilon)
            break;

        // Collect all the results of the collisions
        std::vector<CollisionResult> allCollisions;
        CollisionShapeType controlledShape = controlledActor->getCollisionShapeType();

        
        Vec3 testPos = currentPos + remainingMove;
        controlledActor->setWorldPos(testPos); 
        auto controlledWorldAABB = controlledActor->getWorldAABB();
        auto controlledWorldSphere = controlledActor->getWorldSphere();
        auto controlledWorldOBB = controlledActor->getWorldOBB();

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
				else if (controlledShape == CollisionShapeType::OBB)
					
					collision = CollisionDetector::checkOBBOBB(controlledWorldOBB, OBB::fromAABB(actor->getLocalAABB(), actor->getWorldMatrix()));
                break;
            case CollisionShapeType::Sphere:
                if (controlledShape == CollisionShapeType::AABB)
                    collision = CollisionDetector::checkSphereAABB(actor->getWorldSphere(), controlledWorldAABB);
                else if (controlledShape == CollisionShapeType::Sphere)
                    collision = CollisionDetector::checkSphereSphere(controlledWorldSphere, actor->getWorldSphere());
				else if (controlledShape == CollisionShapeType::OBB)
					collision = CollisionDetector::checkOBBSphere(controlledWorldOBB, actor->getWorldSphere());
                break;
			case CollisionShapeType::OBB:
				if (controlledShape == CollisionShapeType::AABB)
					collision = CollisionDetector::checkOBBOBB(OBB::fromAABB(controlledActor->getLocalAABB(), controlledActor->getWorldMatrix()), actor->getWorldOBB());
				else if (controlledShape == CollisionShapeType::Sphere)
					collision = CollisionDetector::checkOBBSphere(actor->getWorldOBB(), controlledWorldSphere);
				else if (controlledShape == CollisionShapeType::OBB)
					collision = CollisionDetector::checkOBBOBB(controlledWorldOBB, actor->getWorldOBB());
				break;
            default:
                continue;
            }

            // Collect effective collisions
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

        
        controlledActor->setWorldPos(currentPos);

        
        if (allCollisions.empty())
        {
            return remainingMove; 
        }

        // Merge all the normal vectors of the collisions
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

        // Decompose the moving vector
        float dotProduct = Dot(remainingMove, combinedNormal);
        if (dotProduct < 0)
        {
            remainingMove = remainingMove - combinedNormal * dotProduct;
        }

        
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
        auto controlledWorldOBB = controlledActor->getWorldOBB();
        // check 
		CollisionResult collision;
		switch (actor->getCollisionShapeType())
		{
		case CollisionShapeType::AABB:
			if (controlledShape == CollisionShapeType::AABB)
				collision = CollisionDetector::checkAABBAABB(controlledWorldAABB, actor->getWorldAABB());
			else if (controlledShape == CollisionShapeType::Sphere)
				collision = CollisionDetector::checkSphereAABB(controlledWorldSphere, actor->getWorldAABB());
			else if (controlledShape == CollisionShapeType::OBB)
				
				collision = CollisionDetector::checkOBBOBB(controlledWorldOBB, OBB::fromAABB(actor->getLocalAABB(), actor->getWorldMatrix()));
            break;
		case CollisionShapeType::Sphere:
			if (controlledShape == CollisionShapeType::AABB)
				collision = CollisionDetector::checkSphereAABB(actor->getWorldSphere(), controlledWorldAABB);
			else if (controlledShape == CollisionShapeType::Sphere)
				collision = CollisionDetector::checkSphereSphere(controlledWorldSphere, actor->getWorldSphere());
			else if (controlledShape == CollisionShapeType::OBB)
				collision = CollisionDetector::checkOBBSphere(controlledWorldOBB, actor->getWorldSphere());
            break;
		case CollisionShapeType::OBB:
			if (controlledShape == CollisionShapeType::AABB)
				collision = CollisionDetector::checkOBBOBB(OBB::fromAABB(controlledActor->getLocalAABB(), controlledActor->getWorldMatrix()), actor->getWorldOBB());
			else if (controlledShape == CollisionShapeType::Sphere)
				collision = CollisionDetector::checkOBBSphere(actor->getWorldOBB(), controlledWorldSphere);
			else if (controlledShape == CollisionShapeType::OBB)
				collision = CollisionDetector::checkOBBOBB(controlledWorldOBB, actor->getWorldOBB());
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
