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
	std::map<std::string, Actor*>& GetAllActors() 
	{
		return m_actors;
	}
	void AddActor(std::string name, Actor* actor)
	{
		// check if exist actor with same name
		std::string realName = name;
		int i = 0;
		do
		{
			auto findIt = m_actors.find(name);
			if (findIt != m_actors.end())
			{
				realName = name + std::to_string(i);
			}
			else
			{
				m_actors[realName] = actor;
				break;
			}
		} while (true);
		 
		
	}

	virtual void draw() = 0;
};


class TestMap : public Level
{
	

public:
	TestMap();
	virtual void draw() override;


};