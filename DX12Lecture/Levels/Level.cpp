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

	Actor* fpsActor = new FPSActor();
	m_actors["FPSActor"] = fpsActor;
}

void TestMap::draw()
{
	for (auto& pair : m_actors)
	{
		pair.second->draw();
	}
}
