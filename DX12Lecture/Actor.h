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
	std::vector<Matrix> instanceMatrices; // 存储每个实例的世界矩阵
	int instanceCount; // 实例数量
	// 生成随机位置的实例矩阵
	void generateInstanceMatrices(int count, float radius, float height);
public:
	TreeActor(int count = 50);
	virtual void draw() override;
};

class WaterActor : public Actor
{
	StaticMesh* water;

public:
	WaterActor();
	virtual void draw() override;
};
