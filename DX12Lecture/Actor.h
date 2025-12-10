#pragma once
#include "Mesh.h"


class Actor
{
	// collision system

public:
	// draw
	virtual void draw() = 0;
};

class SkyBoxActor: public Actor
{
	StaticMesh* skybox;
public:
	SkyBoxActor();
	virtual void draw() override;
};

class TreeActor : public Actor
{
	StaticMesh* willow;
public:
	TreeActor();
	virtual void draw() override;
};

