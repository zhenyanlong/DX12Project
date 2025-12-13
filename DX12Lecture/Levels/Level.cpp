#include "Level.h"
#include "World.h"


TestMap::TestMap()
{
	World* myWorld = World::Get();
	// create skybox
	Actor* sky = new SkyBoxActor();
	
	m_actors["SkyBox"] = sky;
	
	Actor* tree = new TreeActor();
	m_actors["Tree"] = tree;

	Actor* water = new WaterActor();
	m_actors["Water"] = water;
}

void TestMap::draw()
{
	for (auto& pair : m_actors)
	{
		pair.second->draw();
	}
}
