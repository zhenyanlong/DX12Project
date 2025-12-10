#pragma once
#include "iostream"
#include "map"
#include "Actor.h"
#include "Mesh.h"
class Level
{
protected:
	std::map<std::string, Actor*> m_actors;

	

public:
	// construct
	Level()
	{
		
	}
	~Level()
	{

	}

	virtual void draw() = 0;
};


class TestMap : public Level
{
	

public:
	TestMap();
	virtual void draw() override;


};