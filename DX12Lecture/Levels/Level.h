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

	Actor* GetActor(std::string name)
	{
		auto findIt = m_actors.find(name);
		if (findIt == m_actors.end())
		{
			return nullptr;
		}
		return findIt->second;
	}

	virtual void draw() = 0;
};


class TestMap : public Level
{
	

public:
	TestMap();
	virtual void draw() override;


};