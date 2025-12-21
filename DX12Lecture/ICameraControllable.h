#pragma once
#include "Vec3.h"
#include "Core.h"

// Inherit from a class that can be controlled by the player
class CameraControllable
{
public:
	virtual void updatePos(Vec3 pos) = 0;

	virtual void updateRotation(Vec3 rot) = 0;
	
	virtual void updateRotation(Matrix rotMat) = 0;

	virtual void updateWorldMatrix(Vec3 pos, float yaw, float pitch) = 0;
};