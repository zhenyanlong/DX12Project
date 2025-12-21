#pragma once
#include "iostream"
#include "map"
#include "Actor.h"
#include "Mesh.h"
#include <fstream>
#include <string>
#include "Vec3.h"
class Level
{
protected:
	std::map<std::string, Actor*> m_actors;
	Vec3 m_spawnPoint = Vec3(40.0f, 15.0f, 0.0f); // spawn point

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
			delete it->second;  
			it->second = nullptr;

			
			m_actors.erase(it);

			
		}
	}
	// garbage collection
	void garbageColloection()
	{
		
		for (auto it = m_actors.begin(); it != m_actors.end(); ) {
			if (it->second && it->second->getIsDestroyed()) {
				
				delete it->second;
				it->second = nullptr;

				
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

	public:
		
		void SetSpawnPoint(const Vec3& pos) { m_spawnPoint = pos; }
		Vec3 GetSpawnPoint() const { return m_spawnPoint; }

		
		virtual bool SaveLevel(const std::string& filePath);
		virtual bool LoadLevel(const std::string& filePath);
};


class TestMap : public Level
{
	

public:
	TestMap();
	virtual void draw() override;


};