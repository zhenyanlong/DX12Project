#pragma once
#include "Vec3.h"
#include "Core.h"
#include "ScreenSpaceTriangle.h"
class ScreenSpaceTriangle
{
public:
	PRIM_VERTEX vertices[3];

	

public:
	ScreenSpaceTriangle()
	{
		vertices[0].position = Vec3(0, 1.0f, 0);
		vertices[0].colour = Colour(0, 1.0f, 0);
		vertices[1].position = Vec3(-1.0f, -1.0f, 0);
		vertices[1].colour = Colour(1.0f, 0, 0);
		vertices[2].position = Vec3(1.0f, -1.0f, 0);
		vertices[2].colour = Colour(0, 0, 1.0f);

		
	}
	
	
};

