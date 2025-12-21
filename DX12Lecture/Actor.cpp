#include "Actor.h"
#include "World.h"
#include "Animation/FPSAnimationStateMachine.h"
#include "Animation/EnemyAnimationStateMachine.h"
#define M_PI       3.14159265358979323846

std::map<std::string, Actor::ActorCreator> Actor::m_actorCreators;

void Actor::RegisterActor(const std::string& className, ActorCreator creator) {
	m_actorCreators[className] = creator;
}

Actor* Actor::CreateActorByClassName(const std::string& className) {
	auto it = m_actorCreators.find(className);
	if (it != m_actorCreators.end()) {
		return it->second();
	}
	return nullptr;
}

// Public data serialization of base class
void Actor::SaveBase(std::ofstream& file) const {
	int actorType = static_cast<int>(m_actorType);
	file.write(reinterpret_cast<const char*>(&actorType), sizeof(int));

	Vec3 pos = getWorldPos();
	file.write(reinterpret_cast<const char*>(&pos), sizeof(Vec3));
	Vec3 rot = getWorldRotation();
	file.write(reinterpret_cast<const char*>(&rot), sizeof(Vec3));
	Vec3 scale = getWorldScale();
	file.write(reinterpret_cast<const char*>(&scale), sizeof(Vec3));

	bool isCollidable = m_isCollidable;
	file.write(reinterpret_cast<const char*>(&isCollidable), sizeof(bool));
	int collisionShapeType = static_cast<int>(m_collisionShapeType);
	file.write(reinterpret_cast<const char*>(&collisionShapeType), sizeof(int));

	bool isDestroyed = m_isDestroyed;
	file.write(reinterpret_cast<const char*>(&isDestroyed), sizeof(bool));
}

void Actor::LoadBase(std::ifstream& file) {
	int actorType;
	file.read(reinterpret_cast<char*>(&actorType), sizeof(int));
	m_actorType = static_cast<ActorType>(actorType);

	Vec3 pos, rot, scale;
	file.read(reinterpret_cast<char*>(&pos), sizeof(Vec3));
	setWorldPos(pos);
	file.read(reinterpret_cast<char*>(&rot), sizeof(Vec3));
	setWorldRotation(rot);
	file.read(reinterpret_cast<char*>(&scale), sizeof(Vec3));
	setWorldScale(scale);

	bool isCollidable;
	file.read(reinterpret_cast<char*>(&isCollidable), sizeof(bool));
	m_isCollidable = isCollidable;
	int collisionShapeType;
	file.read(reinterpret_cast<char*>(&collisionShapeType), sizeof(int));
	m_collisionShapeType = static_cast<CollisionShapeType>(collisionShapeType);

	bool isDestroyed;
	file.read(reinterpret_cast<char*>(&isDestroyed), sizeof(bool));
	m_isDestroyed = isDestroyed;
}
namespace {
	struct SkyBoxActorRegistrar {
		SkyBoxActorRegistrar() {
			Actor::RegisterActor("SkyBoxActor", []() { return new SkyBoxActor(); });
		}
	} skyBoxActorRegistrar;
}

SkyBoxActor::SkyBoxActor()
{
	World* myWorld = World::Get();
	skybox = new StaticMesh();
	skybox->CreateFromSphere(myWorld->GetCore(), 64, 64, 5, "Models/Textures/sky_ps.png");
	skybox->SetWorldScaling(Vec3(1000.f, 1000.f, 1000.f));
	skybox->SetWorldRotationRadian(Vec3(M_PI, 0.f, 0.f));
}

void SkyBoxActor::draw()
{
	World* myWorld = World::Get();
	skybox->drawSingle(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

namespace {
	struct TreeActorRegistrar {
		TreeActorRegistrar() {
			Actor::RegisterActor("TreeActor", []() { return new TreeActor(); });
		}
	} treeActorRegistrar;
}

TreeActor::TreeActor(int count, Vec3 transIncrement)
{
	m_instanceCount = count;
	m_transIncrement = transIncrement;

	World* myWorld = World::Get();
	willow = new StaticMesh(myWorld->GetCore(), "Models/willow.gem");
	generateInstanceMatrices(m_instanceCount, m_transIncrement);
}

void TreeActor::generateInstanceMatrices(int count, Vec3 transIncrement)
{
	instanceMatrices.clear();
	
	m_instanceCount = count;
	m_transIncrement = transIncrement;
	for (int i = 0; i < m_instanceCount; i++)
	{
		float scale = 0.05f;
		float rotY = (float)rand() / RAND_MAX * 2 * M_PI;
		Vec3 originPos = willow->GetWorldPos();

		Matrix scaleMat = Matrix::scaling(Vec3(scale, scale, scale));
		Matrix rotMat = Matrix::rotateZ(0.f) * Matrix::rotateY(rotY) * Matrix::rotateX(0.f);
		Matrix transMat = Matrix::translation(Vec3(originPos.x + i * transIncrement.x, originPos.y + i * transIncrement.y, originPos.z + i * transIncrement.z));
		Matrix instanceMat = scaleMat * rotMat * transMat;

		instanceMatrices.push_back(instanceMat);
	}
}

void TreeActor::draw()
{
	World* myWorld = World::Get();
	
	willow->drawInstances(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_INSTANCE_LIGHT_PIPE, myWorld->GetPipelines(), &instanceMatrices, m_instanceCount);
}

namespace {
	struct WaterActorRegistrar {
		WaterActorRegistrar() {
			Actor::RegisterActor("WaterActor", []() { return new WaterActor(); });
		}
	} waterActorRegistrar;
}

WaterActor::WaterActor()
{
	World* myWorld = World::Get();
	water = new StaticMesh();
	water->CreateFromPlane(myWorld->GetCore(),100000, 100000,2000,2000);
}

void WaterActor::draw()
{
	World* myWorld = World::Get();
	water->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_LIGHT_WATER_PIPE, myWorld->GetPipelines());
}

namespace {
	struct FPSActorRegistrar {
		FPSActorRegistrar() {
			Actor::RegisterActor("FPSActor", []() { return new FPSActor(); });
		}
	} fpsActorRegistrar;
}

FPSActor::FPSActor()
{
	World* myWorld = World::Get();
	fps_Mesh = new AnimatedModel(myWorld->GetCore(), "Models/Uzi.gem");
	fps_Mesh->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::Sphere);
	animatedInstance = new AnimationInstance();
	animatedInstance->init(&fps_Mesh->animation, 0);
	m_actorType = ActorType::Player;
	calculateLocalCollisionShape();

	animStateMachine = new FPSAnimationStateMachine(fps_Mesh, animatedInstance);
}

FPSActor::~FPSActor()
{
	delete animStateMachine;
	delete animatedInstance;
	delete fps_Mesh;
}

void FPSActor::draw()
{
	World* myWorld = World::Get();
	fps_Mesh->drawSingle(myWorld->GetCore(), myWorld->GetPSOManager(), ANIM_LIGHT_PIPE, myWorld->GetPipelines(), animatedInstance, myWorld->GetDeltatime(), animStateMachine);
}

void FPSActor::updatePos(Vec3 pos)
{
	fps_Mesh->SetWorldPos(pos);
}

void FPSActor::updateRotation(Vec3 rot)
{
	fps_Mesh->SetWorldRotationRadian(rot);
}

void FPSActor::updateRotation(Matrix rotMat)
{
	fps_Mesh->SetRotationMatrix(rotMat);
}

void FPSActor::updateWorldMatrix(Vec3 pos, float yaw, float pitch)
{

}

void FPSActor::calculateLocalCollisionShape()
{

	if (!fps_Mesh)
		return;


	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto* mesh : fps_Mesh->meshes)
	{

		auto vertices = mesh->getVertices(); 
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	m_localSphere.centre = m_localAABB.getCenter();
}

void FPSActor::OnBeginPlay()
{
}

void FPSActor::OnTick(float dt)
{
	if (animStateMachine) {
		animStateMachine->Update(dt); 
	}
}

namespace {
	struct BoxActorRegistrar {
		BoxActorRegistrar() {
			Actor::RegisterActor("BoxActor", []() { return new BoxActor(); });
		}
	} boxActorRegistrar;
}

BoxActor::BoxActor()
{
	World* myWorld = World::Get();
	box = new StaticMesh(myWorld->GetCore(), "Models/box_024.gem");
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	box->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));

	calculateLocalCollisionShape();
}

void BoxActor::draw()
{
	World* myWorld = World::Get();
	box->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void BoxActor::calculateLocalCollisionShape()
{
	if (!box)
		return;

	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : box->meshes)
	{
		auto vertices = mesh.getVertices(); 
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	m_localSphere.centre = m_localAABB.getCenter();
}

namespace {
	struct GroundActorRegistrar {
		GroundActorRegistrar() {
			Actor::RegisterActor("GroundActor", []() { return new GroundActor(); });
		}
	} groundActorRegistrar;
}

GroundActor::GroundActor()
{
	World* myWorld = World::Get();
	ground = new StaticMesh();
	ground->CreateFromPlane(myWorld->GetCore(), 10000, 20000, 10, 10,"Models/Textures/concrete_floor_damaged_01_diff_1k.png","Models/Textures/concrete_floor_damaged_01_nor_dx_1k.png");
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	calculateLocalCollisionShape();
}

void GroundActor::draw()
{
	World* myWorld = World::Get();
	
	ground->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_LIGHT_PIPE, myWorld->GetPipelines());
}

void GroundActor::calculateLocalCollisionShape()
{
	if (!ground)
		return;

	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : ground->meshes)
	{
		auto vertices = mesh.getVertices(); 
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	m_localSphere.centre = m_localAABB.getCenter();
}

namespace {
	struct ContainerBlueActorRegistrar {
		ContainerBlueActorRegistrar() {
			Actor::RegisterActor("ContainerBlueActor", []() { return new ContainerBlueActor(); });
		}
	} containerBlueActorRegistrar;
}

ContainerBlueActor::ContainerBlueActor()
{
	World* myWorld = World::Get();
	container = new StaticMesh(myWorld->GetCore(), "Models/container_005.gem");

	container->SetWorldRotationRadian(Vec3(0.f, PI / 2, 0.f));
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::OBB);
	calculateLocalCollisionShape();
}

void ContainerBlueActor::draw()
{
	World* myWorld = World::Get();

	container->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void ContainerBlueActor::calculateLocalCollisionShape()
{
	if (!container)
		return;

	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : container->meshes)
	{
		auto vertices = mesh.getVertices(); 
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	m_localSphere.centre = m_localAABB.getCenter();
}

namespace {
	struct BlockActorRegistrar {
		BlockActorRegistrar() {
			Actor::RegisterActor("BlockActor", []() { return new BlockActor(); });
		}
	} blockActorRegistrar;
}

BlockActor::BlockActor()
{
	World* myWorld = World::Get();
	box = new StaticMesh(myWorld->GetCore(), "Models/box_024.gem");
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	box->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));

	calculateLocalCollisionShape();
}

void BlockActor::draw()
{
	World* myWorld = World::Get();

	//box->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void BlockActor::calculateLocalCollisionShape()
{
	if (!box)
		return;

	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : box->meshes)
	{
		auto vertices = mesh.getVertices(); 
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	m_localSphere.centre = m_localAABB.getCenter();
}

namespace {
	struct ObstacleActorRegistrar {
		ObstacleActorRegistrar() {
			Actor::RegisterActor("ObstacleActor", []() { return new ObstacleActor(); });
		}
	} obstacleActorRegistrar;
}

ObstacleActor::ObstacleActor(int count, Vec3 offset)
{
	m_instanceCount = count;
	m_offset = offset;

	World* myWorld = World::Get();
	obstacle = new StaticMesh(myWorld->GetCore(), "Models/obstacle_003.gem");
	//setCollidable(true);
	//setCollisionShapeType(CollisionShapeType::AABB);
	obstacle->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	//box->SetWorldScaling(Vec3(1.1f, 1.1f, 1.1f));
	//calculateLocalCollisionShape();
	generateInstanceMatrices(m_instanceCount, m_offset);
	
}

void ObstacleActor::generateInstanceMatrices(int count, Vec3 offset)
{
	m_instanceCount = count;
	m_offset = offset;
	instanceMatrices.clear();
	
	for (int i = 0; i < count; i++)
	{
		Vec3 scale = obstacle->GetWorldScale();
		Vec3 rot = obstacle->GetWorldRotationRadian();

		float transIncrement = 20.f;
		Vec3 originPos = obstacle->GetWorldPos();

		Matrix scaleMat = Matrix::scaling(Vec3(scale.x, scale.y, scale.z));
		Matrix rotMat = Matrix::rotateZ(rot.z) * Matrix::rotateY(rot.y) * Matrix::rotateX(rot.x);
		Matrix transMat = Matrix::translation(Vec3(originPos.x + i * offset.x, originPos.y + i * offset.y, originPos.z + i * offset.z));
		Matrix instanceMat = scaleMat * rotMat * transMat;

		instanceMatrices.push_back(instanceMat);
	}
}

void ObstacleActor::draw()
{	
	World* myWorld = World::Get();

	obstacle->drawInstances(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_INSTANCE_LIGHT_PIPE, myWorld->GetPipelines(), &instanceMatrices, m_instanceCount);
}

void ObstacleActor::calculateLocalCollisionShape()
{
}

namespace {

	struct GeneralMeshActorRegistrar {
		GeneralMeshActorRegistrar() {
			Actor::RegisterActor("GeneralMeshActor", []() {
				
				return new GeneralMeshActor();
				});
		}
	} generalMeshActorRegistrar; 
}

GeneralMeshActor::GeneralMeshActor(std::string path)
{
	initMesh(path);
}

void GeneralMeshActor::draw()
{
	World* myWorld = World::Get();
	mesh->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void GeneralMeshActor::calculateLocalCollisionShape()
{
	if (!mesh)
		return;

	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto submesh : mesh->meshes)
	{
		auto vertices = submesh.getVertices(); 
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	m_localSphere.centre = m_localAABB.getCenter();
}

void GeneralMeshActor::initMesh(const std::string& path)
{
	m_path = path;

	// Release the old mesh
	if (mesh) {
		delete mesh;
		mesh = nullptr;
	}

	// Recreate the mesh
	World* myWorld = World::Get();
	mesh = new StaticMesh(myWorld->GetCore(), path);
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	mesh->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	calculateLocalCollisionShape();
}



void BulletActor::OnTick(float dt)
{
	World* myWorld = World::Get();
	// Update the lifecycle, automatically destroy upon timeout
	m_lifeTime += dt;
	if (m_lifeTime >= MAX_LIFE_TIME) {
		Destroy();
		return;
	}

	// move
	setWorldPos(getWorldPos() + m_direction * m_speed * dt);
	// check collision
	calculateLocalCollisionShape();
	auto collidableActors = myWorld->getCollidableActors();
	std::vector<Actor*> collisions = CollisionResolver::CheckCollision(this, collidableActors);

	for (auto* actor : collisions)
	{
		if (actor->getActorType() == ActorType::Static)
		{
			Destroy();
		}
		else if (actor->getActorType() == ActorType::Enemy)
		{
			// execute damage api
			dynamic_cast<EnemyActor*>(actor)->animStateMachine->TriggerDeath();

		}
	}
}

BulletActor::BulletActor(const Vec3 pos, const Vec3 dir, float speed, int damage)
{
	World* myWorld = World::Get();
	m_bulletMesh = new StaticMesh();
	m_bulletMesh->CreateFromSphere(myWorld->GetCore(), 64, 64, 10, "Models/Textures/arms_1_Albedo_nh.png");
	// init mesh 
	setWorldPos(pos);
	//setWorldScale(Vec3(1.f, 1.f, 1.f));
	m_direction = dir.normalize();
	m_speed = speed;
	m_damage = damage;
	m_lifeTime = 0.0f;
	m_actorType = ActorType::Bullet;

	
	
	

	// init collision
	//setCollidable(true);
	setCollisionShapeType(CollisionShapeType::Sphere);
	calculateLocalCollisionShape();
}

void BulletActor::draw()
{
	World* myWorld = World::Get();
	m_bulletMesh->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void BulletActor::calculateLocalCollisionShape()
{
	if (!m_bulletMesh)
		return;

	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : m_bulletMesh->meshes)
	{
		auto vertices = mesh.getVertices(); 
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	m_localSphere.centre = m_localAABB.getCenter();
}

namespace {
	struct EnemyActorRegistrar {
		EnemyActorRegistrar() {
			Actor::RegisterActor("EnemyActor", []() { return new EnemyActor(); });
		}
	} enemyActorRegistrar;
}

EnemyActor::EnemyActor()
{
	World* myWorld = World::Get();
	enemy_Mesh = new AnimatedModel(myWorld->GetCore(), "Models/Duck-white.gem");
	enemy_Mesh->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::Sphere);
	animatedInstance = new AnimationInstance();
	animatedInstance->init(&enemy_Mesh->animation, 0);
	m_actorType = ActorType::Enemy;
	calculateLocalCollisionShape();

	animStateMachine = new EnemyAnimationStateMachine(enemy_Mesh, animatedInstance);
}

EnemyActor::~EnemyActor()
{
	delete animStateMachine;
	delete animatedInstance;
	delete enemy_Mesh;
}

void EnemyActor::draw()
{
	World* myWorld = World::Get();
	enemy_Mesh->drawSingle(myWorld->GetCore(), myWorld->GetPSOManager(), ANIM_LIGHT_PIPE, myWorld->GetPipelines(), animatedInstance, myWorld->GetDeltatime(), animStateMachine);
}

void EnemyActor::calculateLocalCollisionShape()
{
	if (!enemy_Mesh)
		return;

	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto* mesh : enemy_Mesh->meshes)
	{
		auto vertices = mesh->getVertices(); 
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	m_localSphere.centre = m_localAABB.getCenter();
}

void EnemyActor::OnBeginPlay()
{
}

void EnemyActor::OnTick(float dt)
{
	if (animStateMachine) {
		animStateMachine->Update(dt); 
	}
	if (animStateMachine->IsDeathFinished())
	{
		Destroy();
	}
}
