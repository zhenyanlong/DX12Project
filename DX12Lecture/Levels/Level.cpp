#include "Level.h"
#include "World.h"
#define PI       3.14159265358979323846

TestMap::TestMap()
{
	World* myWorld = World::Get();
	// create skybox
	Actor* sky = new SkyBoxActor();
	
	m_actors["SkyBox"] = sky;
	
	Actor* tree = new TreeActor(5);
	tree->setWorldPos(Vec3(-45.f, -1.5f, -35.f));
	dynamic_cast<TreeActor*>(tree)->generateInstanceMatrices(5, 50.0f, 5.0f);
	m_actors["Tree"] = tree;

	Actor* water = new WaterActor();
	water->setWorldPos(Vec3(100.f, -10.f, 0.f));
	m_actors["Water"] = water;

	Actor* fpsActor = new FPSActor();
	m_actors["FPSActor"] = fpsActor;

	/*Actor* boxActor = new BoxActor();
	boxActor->setWorldScale(Vec3(0.02f, 0.02f, 0.02f));
	m_actors["BoxActor"] = boxActor;*/

	Actor* boxActor1 = new BoxActor();
	boxActor1->setWorldPos(Vec3(0.f, 20.f, 0.f));
	boxActor1->setWorldRotation(Vec3(0.f, 1.f, 0.f));
	m_actors["BoxActor1"] = boxActor1;

	Actor* boxActor2 = new BoxActor();
	boxActor2->setWorldPos(Vec3(10.f,0.f,33.75f));
	boxActor2->setWorldScale(Vec3(0.025f, 0.025f, 0.025f));
	m_actors["BoxActor2"] = boxActor2;

	Actor* boxActor3 = new BoxActor();
	boxActor3->setWorldPos(Vec3(3.75f, 0.f, 33.75f));
	boxActor3->setWorldScale(Vec3(0.03f, 0.03f, 0.03f));
	m_actors["BoxActor3"] = boxActor3;

	Actor* groundActor = new GroundActor();
	m_actors["GroundActor"] = groundActor;

	Actor* containerActor = new ContainerBlueActor();
	containerActor->setWorldPos(Vec3(0.f, 0.f, 40.f));
	containerActor->setWorldScale(Vec3(0.03f, 0.03f, 0.03f));
	m_actors["ContainerActor"] = containerActor;

	Actor* containerActor1 = new ContainerBlueActor();
	containerActor1->setWorldPos(Vec3(0.f, 0.f, -40.f));
	containerActor1->setWorldScale(Vec3(0.03f, 0.03f, 0.03f));
	m_actors["ContainerActor1"] = containerActor1;

	/*Actor* containerActor1 = new ContainerBlueActor();
	containerActor1->setWorldPos(Vec3(0.f, 0.f, -40.f));
	containerActor1->setWorldScale(Vec3(0.03f, 0.03f, 0.03f));
	m_actors["ContainerActor1"] = containerActor1;*/

	Actor* containerActor2 = new ContainerBlueActor();
	containerActor2->setWorldPos(Vec3(20.f, 0.f, 0.f));
	containerActor2->setWorldRotation(Vec3(0.f, 30.f * PI / 180, 0.f));
	containerActor2->setWorldScale(Vec3(0.03f, 0.03f, 0.03f));
	m_actors["containerActor2"] = containerActor2;

	Actor* containerActor3 = new ContainerBlueActor();
	containerActor3->setWorldPos(Vec3(-20.f, 0.f, 0.f));
	containerActor3->setWorldRotation(Vec3(0.f, -30.f * PI / 180, 0.f));
	containerActor3->setWorldScale(Vec3(0.03f, 0.03f, 0.03f));
	m_actors["containerActor3"] = containerActor3;

	Actor* blockActor = new BlockActor();
	blockActor->setWorldPos(Vec3(50.f, 0.f, 0.f));
	blockActor->setWorldScale(Vec3(0.05, 0.08, 1.f));
	m_actors["BlockActor"] = blockActor;

	Actor* blockActor1 = new BlockActor();
	blockActor1->setWorldPos(Vec3(-45.f, 0.f, 0.f));
	blockActor1->setWorldScale(Vec3(0.05, 0.08, 1.f));
	m_actors["blockActor1"] = blockActor1;

	Actor* houseActor = new GeneralMeshActor("Models/hangar_006.gem");
	houseActor->setWorldPos(Vec3(2.f,0.f,90.f));
	houseActor->setWorldScale(Vec3(0.012f, 0.02f, 0.02f));
	m_actors["houseActor"] = houseActor;

	Actor* houseActor1 = new GeneralMeshActor("Models/hangar_006.gem");
	houseActor1->setWorldPos(Vec3(2.f, 0.f, -100.f));
	houseActor1->setWorldScale(Vec3(0.012f, 0.02f, 0.02f));
	m_actors["houseActor1"] = houseActor1;

	Actor* obstacleActor = new ObstacleActor();
	obstacleActor->setWorldPos(Vec3(45.f, 0.f, -90.f));
	obstacleActor->setWorldScale(Vec3(0.01f, 0.01f, 0.01f));
	obstacleActor->setWorldRotation(Vec3(0.f, PI / 2, 0.f));
	dynamic_cast<ObstacleActor*>(obstacleActor)->generateInstanceMatrices(20, Vec3(0.f, 0.f, 10.f));
	m_actors["ObstacleActor"] = obstacleActor;

	Actor* obstacleActor1 = new ObstacleActor();
	obstacleActor1->setWorldPos(Vec3(-40.f, 0.f, -90.f));
	obstacleActor1->setWorldScale(Vec3(0.02f, 0.02f, 0.02f));
	obstacleActor1->setWorldRotation(Vec3(0.f, PI / 2, 0.f));
	dynamic_cast<ObstacleActor*>(obstacleActor1)->generateInstanceMatrices(10, Vec3(0.f, 0.f, 20.f));
	m_actors["ObstacleActor1"] = obstacleActor1;
	
}

void TestMap::draw()
{
	for (auto& pair : m_actors)
	{
		pair.second->draw();
	}
}
