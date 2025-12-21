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
	dynamic_cast<TreeActor*>(tree)->generateInstanceMatrices(5,Vec3(0.f,0.f,20.f));
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
	boxActor1->setWorldPos(Vec3(0.f, 0.f, 0.f));
	boxActor1->setWorldRotation(Vec3(0.f, 1.f, 0.f));
	boxActor1->setWorldScale(Vec3(0.025f, 0.025f, 0.025f));
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

	Actor* duckactor = new EnemyActor();
	duckactor->setWorldPos(Vec3(30.f, 0.f, -30.f));
	m_actors["duckactor"] = duckactor;

	Actor* duckactor1 = new EnemyActor();
	duckactor1->setWorldPos(Vec3(-30.f, 0.f, -30.f));
	m_actors["duckactor1"] = duckactor1;

	Actor* duckactor2 = new EnemyActor();
	duckactor2->setWorldPos(Vec3(-30.f, 0.f, 40.f));
	m_actors["duckactor2"] = duckactor2;

	Actor* duckactor3 = new EnemyActor();
	duckactor3->setWorldPos(Vec3(0.f, 0.f, 20.f));
	m_actors["duckactor3"] = duckactor3;

	Actor* duckactor4 = new EnemyActor();
	duckactor4->setWorldPos(Vec3(10.f, 0.f, 70.f));
	m_actors["duckactor4"] = duckactor4;
}

void TestMap::draw()
{
	for (auto& pair : m_actors)
	{
		pair.second->draw();
	}
}
bool Level::SaveLevel(const std::string& filePath) {
	std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
	if (!file.is_open()) {
		return false;
	}

	


	file.write(reinterpret_cast<const char*>(&m_spawnPoint), sizeof(Vec3));

	int actorCount = static_cast<int>(m_actors.size());
	file.write(reinterpret_cast<const char*>(&actorCount), sizeof(int));

	for (const auto& pair : m_actors) {
		const std::string& actorName = pair.first;
		Actor* actor = pair.second;

		int nameLen = static_cast<int>(actorName.size());
		file.write(reinterpret_cast<const char*>(&nameLen), sizeof(int));
		file.write(actorName.c_str(), nameLen);

		std::string className = actor->GetClassName();
		int classLen = static_cast<int>(className.size());
		file.write(reinterpret_cast<const char*>(&classLen), sizeof(int));
		file.write(className.c_str(), classLen);

		actor->Save(file);
	}

	file.close();
	return true;
}

bool Level::LoadLevel(const std::string& filePath) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}



	file.read(reinterpret_cast<char*>(&m_spawnPoint), sizeof(Vec3));

	int actorCount;
	file.read(reinterpret_cast<char*>(&actorCount), sizeof(int));

	for (auto& pair : m_actors) {
		delete pair.second;
	}
	m_actors.clear();

	for (int i = 0; i < actorCount; ++i) {
		int nameLen;
		file.read(reinterpret_cast<char*>(&nameLen), sizeof(int));
		std::string actorName(nameLen, '\0');
		file.read(&actorName[0], nameLen);

		int classLen;
		file.read(reinterpret_cast<char*>(&classLen), sizeof(int));
		std::string className(classLen, '\0');
		file.read(&className[0], classLen);

		Actor* actor = Actor::CreateActorByClassName(className);
		if (!actor) {
			continue;
		}

		actor->Load(file);

		m_actors[actorName] = actor;
	}

	file.close();
	return true;
}