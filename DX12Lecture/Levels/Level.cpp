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

	Actor* boxActor = new BoxActor();
	m_actors["BoxActor"] = boxActor;

	Actor* boxActor1 = new BoxActor();
	boxActor1->setWorldPos(Vec3(0.f, 20.f, 0.f));
	boxActor1->setWorldRotation(Vec3(0.f, 1.f, 0.f));
	m_actors["BoxActor1"] = boxActor1;

	Actor* groundActor = new GroundActor();
	m_actors["GroundActor"] = groundActor;
}

void TestMap::draw()
{
	for (auto& pair : m_actors)
	{
		pair.second->draw();
	}
}
