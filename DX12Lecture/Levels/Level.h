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
			auto findIt = m_actors.find(realName);
			if (findIt != m_actors.end())
			{
				realName = name + std::to_string(i);
				i++;
			}
			else
			{
				m_actors[realName] = actor;
				break;
			}
		} while (true);
		 
		//m_actors[name] = actor;
	}
	// remove single actor
	void RemoveActor(Actor* actor)
	{
		auto it = std::find_if(m_actors.begin(), m_actors.end(),
			[actor](const auto& pair) {
				return pair.second == actor;
			});
		if (it != m_actors.end()) {
			delete it->second;  // 先删除对象
			it->second = nullptr;

			// 从map中移除元素
			m_actors.erase(it);

			
		}
	}
	// garbage collection
	void garbageColloection()
	{
		
		for (auto it = m_actors.begin(); it != m_actors.end(); /* 这里不递增 */) {
			if (it->second && it->second->getIsDestroyed()) {
				// 如果map拥有所有权，先删除对象
				delete it->second;
				it->second = nullptr;

				// 从map中删除，并获取下一个迭代器
				it = m_actors.erase(it);
			}
			else {
				++it;
			}
		}
	}
	// execute begin play
	void BeginPlayInLevel()
	{
		for (auto pair : m_actors)
		{
			pair.second->BeginPlay();
		}
	}
	// execute Tick
	void TickInLevel(float dt)
	{
		for (auto pair : m_actors)
		{
			pair.second->Tick(dt);
		}
	}
	virtual void draw() = 0;
};


class TestMap : public Level
{
	

public:
	TestMap();
	virtual void draw() override;


};